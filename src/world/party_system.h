/*
===========================================================================

  Copyright (c) 2025 LandSandBoat Dev Teams

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses/

===========================================================================
*/

#pragma once

#include "common/party.h"
#include "ipc_server.h"
#include "map/ipc_client.h"
#include "world_server.h"

#include <common/ipc.h>
#include <ipc_stubs.h>

class Party
{
    uint32 m_PartyId = 0;
    // TODO: Vector like the map process counterpart?
    std::array<std::optional<PartyMember>, 6> m_Members{};
    uint32                                    m_LeaderUniqueNo        = 0;
    uint32                                    m_QuarterMasterUniqueNo = 0;
    uint32                                    m_SyncTargetUniqueNo    = 0;
    bool                                      dirty                   = true;
    IPCServer*                                m_IpcServer;

public:
    Party(uint32 leaderId, IPCServer* ipcServer)
    : m_LeaderUniqueNo(leaderId)
    , m_IpcServer(ipcServer)
    {
        m_PartyId = m_LeaderUniqueNo;
    }

    void SetDirty(bool isDirty)
    {
        dirty = isDirty;
    }

    bool IsDirty() const
    {
        return dirty;
    }

    uint32 GetPartyId() const
    {
        return m_PartyId;
    }

    size_t GetMemberCount() const
    {
        size_t result = 0;
        for (auto& memberSlot : m_Members)
        {
            if (memberSlot.has_value())
            {
                result += 1;
            }
        }

        return result;
    }

    std::vector<const PartyMember*> GetMembers() const
    {
        std::vector<const PartyMember*> result;

        for (const auto& memberSlot : m_Members)
        {
            if (memberSlot.has_value())
            {
                result.push_back(&*memberSlot);
            }
        }

        return result;
    }

    auto ForEachMember(const std::function<void(const PartyMember&)>& func) const -> void
    {
        for (const auto& memberSlot : m_Members)
        {
            if (memberSlot.has_value())
            {
                func(*memberSlot);
            }
        }
    }

    auto ForEachMemberInZone(const uint16 zoneId, const std::function<void(const PartyMember&)>& func) const -> void
    {
        for (const auto& memberSlot : m_Members)
        {
            if (memberSlot.has_value())
            {
                if (memberSlot->GetZone() == zoneId)
                {
                    func(*memberSlot);
                }
            }
        }
    }

    auto GetLeader() const -> const PartyMember*
    {
        const auto isLeader = [&](const std::optional<PartyMember>& opt_member) -> bool
        {
            if (!opt_member.has_value())
            {
                return false;
            }

            return opt_member.value().GetId() == m_LeaderUniqueNo;
        };

        const auto leader = std::find_if(m_Members.begin(), m_Members.end(), isLeader);

        return leader != m_Members.end() ? &(leader->value()) : nullptr;
    }

    auto GetQuartermaster() const -> const PartyMember*
    {
        if (m_QuarterMasterUniqueNo == 0)
        {
            return nullptr;
        }

        const auto isQm = [&](const std::optional<PartyMember>& opt_member) -> bool
        {
            if (!opt_member.has_value())
            {
                return false;
            }

            return opt_member.value().GetId() == m_QuarterMasterUniqueNo;
        };

        const auto quartermaster = std::find_if(m_Members.begin(), m_Members.end(), isQm);
        return quartermaster != m_Members.end() ? &(quartermaster->value()) : nullptr;
    }

    auto GetSyncTarget() const -> const PartyMember*
    {
        if (m_SyncTargetUniqueNo == 0)
        {
            return nullptr;
        }

        const auto isSync = [&](const std::optional<PartyMember>& opt_member) -> bool
        {
            if (!opt_member.has_value())
            {
                return false;
            }

            return opt_member.value().GetId() == m_SyncTargetUniqueNo;
        };

        const auto sync = std::find_if(m_Members.begin(), m_Members.end(), isSync);
        return sync != m_Members.end() ? &(sync->value()) : nullptr;
    }

    bool SetLeader(uint32_t UniqueNo)
    {
        for (size_t i = 0; i < m_Members.size(); ++i)
        {
            auto& memberSlot = m_Members[i];
            if (memberSlot && memberSlot->GetId() == UniqueNo)
            {
                m_LeaderUniqueNo = UniqueNo;
                m_PartyId        = UniqueNo;
                ShowInfoFmt("Leader set to UniqueNo: {}, changed PartyId", UniqueNo);
                SetDirty(true);
                return true;
            }
        }
        ShowWarningFmt("Member with UniqueNo: {} not found in party", UniqueNo);
        return false;
    }

    bool SetQuartermaster(uint32_t UniqueNo)
    {
        if (UniqueNo == 0)
        {
            m_QuarterMasterUniqueNo = 0;
            ShowInfo("Quartermaster removed");
            SetDirty(true);
            return true;
        }

        for (size_t i = 0; i < m_Members.size(); ++i)
        {
            auto& memberSlot = m_Members[i];
            if (memberSlot && memberSlot->GetId() == UniqueNo)
            {
                m_QuarterMasterUniqueNo = UniqueNo;
                ShowInfoFmt("Quartermaster set to UniqueNo: {}", UniqueNo);
                SetDirty(true);
                return true;
            }
        }

        ShowWarningFmt("Member with UniqueNo: {} not found in party", UniqueNo);
        return false;
    }

    bool SetSyncTarget(uint32_t UniqueNo)
    {
        if (UniqueNo == 0)
        {
            m_SyncTargetUniqueNo = 0;
            ShowInfo("Sync target removed");
            SetDirty(true);
            return true;
        }

        for (size_t i = 0; i < m_Members.size(); ++i)
        {
            auto& memberSlot = m_Members[i];
            if (memberSlot && memberSlot->GetId() == UniqueNo)
            {
                m_SyncTargetUniqueNo = UniqueNo;
                ShowInfoFmt("Sync target set to UniqueNo: {}", UniqueNo);
                SetDirty(true);
                return true;
            }
        }
        ShowWarningFmt("Member with UniqueNo: {} not found in party", UniqueNo);
        return false;
    }

    bool AddMember(uint32_t UniqueNo, PartyMemberType type, const uint32 ZoneId)
    {
        for (const auto& memberSlot : m_Members)
        {
            if (memberSlot.has_value() && memberSlot->GetId() == UniqueNo)
            {
                ShowWarningFmt("Member with UniqueNo: {} already exists in the party", UniqueNo);
                return false;
            }
        }

        for (auto& memberSlot : m_Members)
        {
            if (!memberSlot.has_value())
            {
                // Capture PC names. Not relevant for trusts.
                std::string charName = "";
                if (type == PartyMemberType::Player)
                {
                    const auto rset = db::preparedStmt("SELECT charname FROM chars WHERE charid = ?");
                    if (rset && rset->rowsCount() && rset->next())
                    {
                        charName = rset->get<std::string>("charname");
                    }
                }

                memberSlot = PartyMember{ UniqueNo, type, ZoneId, charName };
                ShowInfoFmt("Added member {} ({}) (type {})", charName, UniqueNo, static_cast<uint8>(type));
                SetDirty(true);
                return true;
            }
        }

        ShowWarningFmt("Party is full, cannot add member with UniqueNo: {}", UniqueNo);
        return false;
    }

    bool RemoveMember(uint32 UniqueNo)
    {
        for (const auto& member : GetMembers())
        {
            if (member->GetId() == UniqueNo)
            {
                for (auto& memberSlot : m_Members)
                {
                    if (memberSlot.has_value() && memberSlot->GetId() == UniqueNo)
                    {
                        memberSlot.reset();
                    }
                }

                // Removing leader, pass to someone else.
                // TODO: Do by timer
                if (m_LeaderUniqueNo == member->GetId())
                {
                    for (size_t j = 0; j < m_Members.size(); ++j)
                    {
                        if (m_Members[j])
                        {
                            m_LeaderUniqueNo = m_Members[j]->GetId();
                            m_PartyId        = m_LeaderUniqueNo;
                            ShowInfoFmt("Leader reassigned to UniqueNo: {}", m_Members[j]->GetId());
                            break;
                        }
                    }
                }

                if (m_QuarterMasterUniqueNo == UniqueNo)
                    SetQuartermaster(0);

                if (m_SyncTargetUniqueNo == UniqueNo)
                    SetSyncTarget(0);

                ShowInfoFmt("Removed member with UniqueNo: {}", UniqueNo);
                SetDirty(true);
                return true;
            }
        }

        ShowWarningFmt("Member with UniqueNo: {} not found in party", UniqueNo);
        return false;
    }

    ipc::PartyUpdate AsPartyUpdate() const
    {
        auto mMembers = std::vector<PartyMemberData>{};
        for (const auto& member : GetMembers())
        {
            mMembers.push_back(member->Serializable());
        }

        return ipc::PartyUpdate{
            .partyId               = m_PartyId,
            .leaderUniqueNo        = m_LeaderUniqueNo,
            .quartermasterUniqueNo = m_QuarterMasterUniqueNo,
            .syncTargetUniqueNo    = m_SyncTargetUniqueNo,
            .members               = mMembers,
        };
    }
};

class PartySystem
{
public:
    PartySystem(WorldServer& worldServer)
    : worldServer_(worldServer)
    {
    }

    ~PartySystem() = default;

    Party* GetParty(const uint32 partyId)
    {
        const auto it = m_Parties.find(partyId);
        return it != m_Parties.end() ? &it->second : nullptr;
    }

    void NotifyIppForParty(const uint32 partyId, const auto& message, const uint16 zoneId) const
    {
        if (const auto it = m_Parties.find(partyId); it != m_Parties.end())
        {
            for (const auto& member : it->second.GetMembers())
            {
                if (member->GetZone() == zoneId)
                {
                    worldServer_.ipcServer_->rerouteMessageToCharId(member->GetId(), message);
                }
            }
        }
    }

    void NotifyIppForParty(const uint32 partyId, const auto& message) const
    {
        worldServer_.ipcServer_->rerouteMessageToPartyMembers(partyId, message);
    }

    template <typename Func, typename... Args>
    bool ModifyParty(uint32 partyId, Func&& func, Args&&... args)
    {
        const auto it = m_Parties.find(partyId);
        if (it == m_Parties.end())
        {
            ShowErrorFmt("Party with ID {} not found", partyId);
            return false;
        }

        const bool result = (it->second.*func)(std::forward<Args>(args)...);

        if (it->second.IsDirty())
        {
            BroadcastPartyUpdate(it->second);
        }

        return result;
    }

    void BroadcastPartyUpdate(Party& party) const
    {
        if (party.IsDirty())
        {
            for (const auto& member : party.GetMembers())
            {
                if (member->GetType() != PartyMemberType::Player)
                {
                    continue;
                }

                // Temporary hack to make the search server work
                db::preparedStmt("INSERT INTO accounts_parties (charid, partyid, allianceid, partyflag) VALUES (?, ?, ?, ?)"
                                 "ON DUPLICATE KEY UPDATE "
                                 "partyid = VALUES(partyid), "
                                 "partyflag = VALUES(partyflag)",
                                 member->GetId(),
                                 party.GetPartyId(),
                                 0,
                                 member == party.GetLeader() ? 4 : 0);
            }
            ShowInfoFmt("Notifying map servers that party {} is dirty.", party.GetPartyId());
            NotifyIppForParty(party.GetPartyId(), party.AsPartyUpdate());
            party.SetDirty(false);
        }
    }

    void Dump()
    {
        for (auto& [partyId, party] : m_Parties)
        {
            ShowInfoFmt("Party {}: {}/6", partyId, party.GetMemberCount());
            auto pLeader        = party.GetLeader();
            auto pQuarterMaster = party.GetQuartermaster();
            auto pSyncTarget    = party.GetSyncTarget();

            for (const auto& member : party.GetMembers())
            {
                if (member == pLeader)
                {
                    ShowInfoFmt("  Leader: {} (joined {} ago)", member->GetId(), member->GetTimeSinceJoined());
                }
                else if (member == pQuarterMaster)
                {
                    ShowInfoFmt("  Quartermaster: {} (joined {} ago)", member->GetId(), member->GetTimeSinceJoined());
                }
                else if (member == pSyncTarget)
                {
                    ShowInfoFmt("  Sync Target: {} (joined {} ago)", member->GetId(), member->GetTimeSinceJoined());
                }
                else
                {
                    ShowInfoFmt("  Member: {} (joined {}s ago) (type {})", member->GetId(), member->GetTimeSinceJoined(), static_cast<uint8>(member->GetType()));
                }
            }
        }
    }

    bool PartyCreate(const IPP& ipp, const ipc::PartyCreate& message)
    {
        auto [it, inserted] = this->m_Parties.emplace(message.charId, Party(message.charId, worldServer_.ipcServer_.get()));
        it->second.AddMember(message.charId, PartyMemberType::Player, message.zoneId);
        ShowInfoFmt("Party created with charId: {}", message.charId);
        return true;
    }

    bool PartyAddMember(const IPP& ipp, const ipc::PartyAddMember& message)
    {
        return ModifyParty(message.partyId, &Party::AddMember, message.charId, message.type, message.zoneId);
    }

    bool PartyRemoveMember(const IPP& ipp, const ipc::PartyRemoveMember& message)
    {
        const auto& party        = this->m_Parties.at(message.partyId);
        const auto  syncTargetId = party.GetSyncTarget() ? party.GetSyncTarget()->GetId() : 0;

        const bool res = ModifyParty(message.partyId, &Party::RemoveMember, message.charId);
        if (res)
        {
            // If we just removed the sync, we need to notify the party members
            // TODO: this is a mess and should be handled better
            if (syncTargetId == message.charId)
            {
                PartySetSyncTarget(ipp, ipc::PartySetSyncTarget{ .partyId = message.partyId, .charId = 0, .reason = MsgStd::LevelSyncRemoveLeftParty });
            }

            // Notify the player they've been kicked.
            worldServer_.ipcServer_->rerouteMessageToCharId(message.charId, ipc::PlayerKick{ .victimId = message.charId });
        }

        if (const auto it = m_Parties.find(message.partyId); it != m_Parties.end())
        {
            // If the leader leaves, then the party is disbanded
            if (it->second.GetMembers().size() == 0)
            {
                PartyDisband(ipp, ipc::PartyDisband{ .partyId = message.partyId });
            }
        }
        // TODO: If a given map server no longer has any member, they should be notified they need to clear the entry
        // Alternatively, this could be handled on the map process itself.
        return res;
    }

    bool PartyDisband(const IPP& ipp, const ipc::PartyDisband& message)
    {
        if (const auto it = m_Parties.find(message.partyId); it != m_Parties.end())
        {
            // If leader requested breaking the PT, but we still have members, process them first.
            if (const auto members = it->second.GetMembers(); members.size() != 0)
            {
                for (auto& member : members)
                {
                    PartyRemoveMember(ipp, ipc::PartyRemoveMember{ .partyId = message.partyId, .charId = member->GetId() });
                }
            }
        }

        this->m_Parties.erase(message.partyId);
        ShowInfoFmt("Party disbanded with partyId: {}", message.partyId);

        // Notify map servers that the party should no longer be tracked
        // TODO: Store IPPs we've interacted with and only send to those.
        worldServer_.ipcServer_->broadcastMessage(message);
        return true;
    }

    bool PartySetLeader(const IPP& ipp, const ipc::PartySetLeader& message)
    {
        auto&      party       = this->m_Parties.at(message.partyId);
        const auto oldLeaderId = party.GetLeader()->GetId();

        if (const auto newLeaderId = message.charId; party.SetLeader(newLeaderId))
        {
            // From that point on, the underlying leader and partyId have been modified
            // Find and extract the entry under the old leader ID
            if (auto partyEntry = this->m_Parties.extract(oldLeaderId); !partyEntry.empty())
            {
                // Insert it back under the new leader ID
                partyEntry.key() = newLeaderId;
                this->m_Parties.insert(std::move(partyEntry));
                ShowInfoFmt("Party leader set with partyId: {}", message.partyId);
            }

            // Tell map servers to update the partyId. This must happen before the next update.
            worldServer_.ipcServer_->rerouteMessageToPartyMembers(newLeaderId, ipc::PartyChangeId{ .formerId = oldLeaderId, .newId = newLeaderId });

            // Broadcast an update to force the map servers to push packets to the clients
            BroadcastPartyUpdate(party);
            return true;
        }

        return false;
    }

    bool PartySetQuartermaster(const IPP& ipp, const ipc::PartySetQuartermaster& message)
    {
        return ModifyParty(message.partyId, &Party::SetQuartermaster, message.charId);
    }

    bool PartySetSyncTarget(const IPP& ipp, const ipc::PartySetSyncTarget& message)
    {
        const auto& party        = this->m_Parties.at(message.partyId);
        const auto  oldSync      = party.GetSyncTarget();
        uint32      syncTargetId = 0;

        if (message.charId == 0 && !message.charName.empty())
        {
            const auto rset = db::preparedStmt("SELECT charid FROM chars WHERE charname = ?",
                                               message.charName);
            if (rset && rset->rowsCount() && rset->next())
            {
                syncTargetId = rset->get<uint32>("charid");
                // TODO:if (PChar->GetMLevel() < 10)
                // {
                //     ((CCharEntity*)GetLeader())->pushPacket<CMessageBasicPacket>((CCharEntity*)GetLeader(), (CCharEntity*)GetLeader(), 0, 10, MsgStd::LevelSyncDesigneeBelowMin);
                //     return;
                // }
                // else if (PChar->getZone() != GetLeader()->getZone())
                // {
                //     ((CCharEntity*)GetLeader())->pushPacket<CMessageBasicPacket>((CCharEntity*)GetLeader(), (CCharEntity*)GetLeader(), 0, 0, MsgStd::LevelSyncDesigneeInOtherArea);
                //     return;
                // }
                // for (auto& member : members)
                // {
                //     if (member->StatusEffectContainer->HasStatusEffect({ EFFECT_LEVEL_RESTRICTION, EFFECT_LEVEL_SYNC, EFFECT_SJ_RESTRICTION, EFFECT_CONFRONTATION, EFFECT_BATTLEFIELD }))
                //     {
                //         ((CCharEntity*)GetLeader())->pushPacket<CMessageBasicPacket>((CCharEntity*)GetLeader(), (CCharEntity*)GetLeader(), 0, 0, MsgStd::LevelSyncPreventedByStatus);
                //         return;
                //     }
                // }
            }
            else
            {
                ShowErrorFmt("PartySetSyncTarget: Unable to find charId for charName: {}", message.charName);
                return false;
            }
        }
        else
        {
            syncTargetId = message.charId;
        }

        const bool res = ModifyParty(message.partyId, &Party::SetSyncTarget, syncTargetId);

        // If a reason was provided for disabling, we need to stream it to certain party members
        if (syncTargetId == 0 && static_cast<uint16>(message.reason) != 0)
        {
            switch (message.reason)
            {
                case MsgStd::LevelSyncRemoveTooFewMembers:
                    // This occurs when there is not enough members in the synced zone
                    party.ForEachMemberInZone(oldSync->GetZone(), [&](const PartyMember& member)
                                              { worldServer_.ipcServer_->rerouteMessageToCharId(member.GetId(), ipc::MessageStandard{
                                                                                                                    .recipientId = member.GetId(),
                                                                                                                    .message     = MsgStd::LevelSyncRemoveTooFewMembers,
                                                                                                                }); });
                    break;
                case MsgStd::LevelSyncRemoveLeftParty:
                    // This occurs when the sync leaves the party
                    party.ForEachMemberInZone(oldSync->GetZone(), [&](const PartyMember& member)
                                              { worldServer_.ipcServer_->rerouteMessageToCharId(member.GetId(), ipc::MessageStandard{
                                                                                                                    .recipientId = member.GetId(),
                                                                                                                    .message     = MsgStd::LevelSyncRemoveLeftParty,
                                                                                                                }); });
                    break;
                case MsgStd::LevelSyncRemoveIneligibleExp:
                    // This occurs when the character is blocked by genkai
                    party.ForEachMemberInZone(oldSync->GetZone(), [&](const PartyMember& member)
                                              { worldServer_.ipcServer_->rerouteMessageToCharId(member.GetId(), ipc::MessageStandard{
                                                                                                                    .recipientId = member.GetId(),
                                                                                                                    .message     = MsgStd::LevelSyncRemoveIneligibleExp,
                                                                                                                }); });
                    break;
                case MsgStd::LevelSyncDeactivateLeftArea:
                    // This occurs when the character leaves the area
                    party.ForEachMemberInZone(oldSync->GetZone(), [&](const PartyMember& member)
                                              { worldServer_.ipcServer_->rerouteMessageToCharId(member.GetId(), ipc::MessageStandard{
                                                                                                                    .recipientId = member.GetId(),
                                                                                                                    .message     = MsgStd::LevelSyncDeactivateLeftArea,
                                                                                                                }); });
                    break;
                default:
                    break;
            }
        }

        return res;
    }

private:
    WorldServer&                      worldServer_;
    std::unordered_map<uint16, Party> m_Parties;
};
