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

#include "common/party/base.h"
#include "common/party/member.h"
#include "ipc_server.h"
#include "map/ipc_client.h"
#include "world_server.h"

#include <common/ipc.h>
#include <ipc_stubs.h>

// Retrieve a couple of information about a character from database to make decisions.

class WorldParty : public PartyBase
{
    IPCServer* m_IpcServer;

public:
    WorldParty(const ipc::PartyUpdate& message)
    : PartyBase(message)
    {
        for (auto& member : message.members)
        {
            m_Members.emplace_back(member);
        }
    }

    WorldParty(uint32 _LeaderUniqueNo)
    : PartyBase(_LeaderUniqueNo)
    {
    }

    bool setMemberZone(const uint32 charId, const uint16 zoneId)
    {
        if (getSyncTarget() && getSyncTarget()->get().getId() == charId)
        {
            setSyncTarget(0);
        }

        if (zoneId == 0xFFFF) // Logging out
        {
            return removeMember(charId);
        }

        for (auto& member : getMembers())
        {
            if (member.getId() == charId)
            {
                member.setZone(zoneId);
                setDirty(true);
                return true;
            }
        }

        return false;
    }

    bool setLeader(uint32_t UniqueNo)
    {
        for (auto& member : getMembers())
        {
            if (member.getId() == UniqueNo)
            {
                m_LeaderUniqueNo = UniqueNo;
                m_PartyId        = UniqueNo;
                ShowInfoFmt("Leader set to UniqueNo: {}, changed PartyId", UniqueNo);
                setDirty(true);
                return true;
            }
        }

        ShowWarningFmt("Member with UniqueNo: {} not found in party", UniqueNo);
        return false;
    }

    bool setQuartermaster(uint32_t UniqueNo)
    {
        if (UniqueNo == 0)
        {
            m_QuartermasterUniqueNo = 0;
            ShowInfo("Quartermaster removed");
            setDirty(true);
            return true;
        }

        for (auto& member : getMembers())
        {
            if (member.getId() == UniqueNo)
            {
                m_QuartermasterUniqueNo = UniqueNo;
                ShowInfoFmt("Quartermaster set to UniqueNo: {}", UniqueNo);
                setDirty(true);
                return true;
            }
        }

        ShowWarningFmt("Member with UniqueNo: {} not found in party", UniqueNo);
        return false;
    }

    bool setSyncTarget(uint32_t UniqueNo)
    {
        if (UniqueNo == 0)
        {
            m_SyncTargetUniqueNo = 0;
            ShowInfo("Sync target removed");
            setDirty(true);
            return true;
        }

        for (auto& member : getMembers())
        {
            if (member.getId() == UniqueNo)
            {
                m_SyncTargetUniqueNo = UniqueNo;
                ShowInfoFmt("Sync target set to UniqueNo: {}", UniqueNo);
                setDirty(true);
                return true;
            }
        }

        ShowWarningFmt("Member with UniqueNo: {} not found in party", UniqueNo);
        return false;
    }

    bool addMember(uint32_t UniqueNo, PartyMemberType type, const uint32 ZoneId)
    {
        for (const auto& member : getMembers())
        {
            if (member.getId() == UniqueNo)
            {
                ShowWarningFmt("Member with UniqueNo: {} already exists in the party", UniqueNo);
                return false;
            }
        }

        if (!isFull())
        {
            // Capture PC names. Not relevant for trusts.
            std::string charName = "";
            if (type == PartyMemberType::Player)
            {
                const auto rset = db::preparedStmt("SELECT charname FROM chars WHERE charid = ?", UniqueNo);
                if (rset && rset->rowsCount() && rset->next())
                {
                    charName = rset->get<std::string>("charname");
                }
            }

            m_Members.emplace_back(UniqueNo, type, ZoneId, charName, std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
            ShowInfoFmt("Added member {} ({}) (type {})", charName, UniqueNo, static_cast<uint8>(type));
            setDirty(true);
            return true;
        }

        ShowWarningFmt("Party is full, cannot add member with UniqueNo: {}", UniqueNo);
        return false;
    }

    bool removeMember(uint32 UniqueNo)
    {
        for (const auto& member : getMembers())
        {
            if (member.getId() == UniqueNo)
            {
                std::erase_if(m_Members, [UniqueNo](const PartyMember& vecMember)
                              { return vecMember.getId() == UniqueNo; });

                // Leader is being removed, pass to someone else.
                // If no eligible member, we need to disband the party
                if (m_LeaderUniqueNo == UniqueNo)
                {
                    if (!m_Members.empty())
                    {
                        // Find member with oldest JoinedTime
                        const auto oldest = std::min_element(m_Members.begin(), m_Members.end(),
                                                             [](const PartyMember& a, const PartyMember& b)
                                                             {
                                                                 return a.getTimeSinceJoined() < b.getTimeSinceJoined();
                                                             });

                        m_LeaderUniqueNo = oldest->getId();
                        m_PartyId        = m_LeaderUniqueNo;
                        ShowInfoFmt("Leader reassigned to UniqueNo: {}", oldest->getId());
                    }
                    else
                    {
                        // Handle empty members case
                        m_LeaderUniqueNo = 0; // or some default/invalid value
                        m_PartyId        = 0;
                        ShowInfoFmt("No members available for leader reassignment");
                    }
                }

                if (m_QuartermasterUniqueNo == UniqueNo)
                    setQuartermaster(0);

                if (m_SyncTargetUniqueNo == UniqueNo)
                    setSyncTarget(0);

                ShowInfoFmt("Removed member with UniqueNo: {}", UniqueNo);
                setDirty(true);
                return true;
            }
        }

        ShowWarningFmt("Member with UniqueNo: {} not found in party", UniqueNo);
        return false;
    }
};

class PartySystem
{
public:
    PartySystem(WorldServer& worldServer)
    : m_WorldServer(worldServer)
    {
    }

    ~PartySystem() = default;

    WorldParty* getParty(const uint32 partyId)
    {
        const auto it = m_Parties.find(partyId);
        return it != m_Parties.end() ? &it->second : nullptr;
    }

    void notifyIppForParty(const uint32 partyId, const auto& message, const uint16 zoneId) const
    {
        if (const auto it = m_Parties.find(partyId); it != m_Parties.end())
        {
            for (const auto& member : it->second.getMembers())
            {
                if (member.getZone() == zoneId)
                {
                    m_WorldServer.ipcServer_->rerouteMessageToCharId(member.getId(), message);
                }
            }
        }
    }

    void notifyIppForParty(const uint32 partyId, const auto& message) const
    {
        m_WorldServer.ipcServer_->rerouteMessageToPartyMembers(partyId, message);
    }

    template <typename Func, typename... Args>
    bool modifyParty(uint32 partyId, Func&& func, Args&&... args)
    {
        const auto it = m_Parties.find(partyId);
        if (it == m_Parties.end())
        {
            ShowErrorFmt("Party with ID {} not found", partyId);
            return false;
        }

        const auto result = (it->second.*func)(std::forward<Args>(args)...);

        if (it->second.isDirty())
        {
            broadcastPartyUpdate(it->second);
        }

        return result;
    }

    void broadcastPartyUpdate(WorldParty& party) const
    {
        if (party.isDirty())
        {
            for (const auto& member : party.getMembers())
            {
                if (member.getType() != PartyMemberType::Player)
                {
                    continue;
                }

                // Temporary hack to make the search server work
                db::preparedStmt("INSERT INTO accounts_parties (charid, partyid, allianceid, partyflag) VALUES (?, ?, ?, ?)"
                                 "ON DUPLICATE KEY UPDATE "
                                 "partyid = VALUES(partyid), "
                                 "partyflag = VALUES(partyflag)",
                                 member.getId(),
                                 party.getPartyId(),
                                 0,
                                 party.getFlagsForMember(party.getLeader().value()));
            }

            ShowInfoFmt("Notifying map servers that party {} is dirty.", party.getPartyId());
            notifyIppForParty(party.getPartyId(), party.asIpcUpdate());
            party.setDirty(false);
        }
    }

    void dump()
    {
        for (auto& [partyId, party] : m_Parties)
        {
            ShowInfoFmt("Party {}: {}/6", partyId, party.getMemberCount());
            const auto pLeader        = party.getLeader();
            const auto pQuarterMaster = party.getQuartermaster();
            const auto pSyncTarget    = party.getSyncTarget();

            //            for (const auto& member : party.getMembers())
            //            {
            ////                if (&member == &pLeader.value())
            ////                {
            ////                    ShowInfoFmt("  Leader: {} (joined {} ago)", member.getId(), member.getTimeSinceJoined());
            ////                }
            ////                else if (&member == pQuarterMaster)
            ////                {
            ////                    ShowInfoFmt("  Quartermaster: {} (joined {} ago)", member.getId(), member.getTimeSinceJoined());
            ////                }
            ////                else if (&member == pSyncTarget)
            ////                {
            ////                    ShowInfoFmt("  Sync Target: {} (joined {} ago)", member.getId(), member.getTimeSinceJoined());
            ////                }
            ////                else
            ////                {
            ////                    ShowInfoFmt("  Member: {} (joined {}s ago) (type {})", member.getId(), member.getTimeSinceJoined(), static_cast<uint8>(member.getType()));
            ////                }
            //            }
        }
    }

    bool handle_CharZoneOut(const IPP& ipp, const ipc::CharZoneOut& message)
    {
        // Find any party with the character
        // TODO: Could use a reverse lookup map or the character cache
        // clang-format off
        const auto it = std::find_if(m_Parties.begin(), m_Parties.end(), [&](auto& entry)
        {
            auto& party  = entry.second;
            auto members = party.getMembers();
            return std::any_of(members.begin(), members.end(), [&](auto& member)
            {
                return member.getId() == message.charId;
            });
        });
        // clang-format on

        if (it != m_Parties.end())
        {
            const auto& party      = it->second;
            const auto  syncTarget = party.getSyncTarget();
            // Sync target is zoning out, remove sync
            if (syncTarget && syncTarget.value().get().getId() == message.charId)
            {
                ShowInfoFmt("Sync target is zoning out. Removing sync target");
                handle_PartySetSyncTarget(ipp, ipc::PartySetSyncTarget{ .partyId = party.getPartyId(), .charId = 0, .reason = MsgStd::LevelSyncDeactivateLeftArea });
            }
            else if (syncTarget)
            {
                // Someone is zoning out, check if enough members are left in the sync zone.
                const uint16 syncZone      = syncTarget.value().get().getZone();
                uint8        membersInZone = 0;

                // clang-format off
                party.ForEveryMember({ .zoneId = syncZone }, [&](const PartyMember& member)
                {
                    if (member.getId() != message.charId)
                    {
                        membersInZone++;
                    }
                });
                // clang-format on

                // Not enough players left, disable sync
                if (membersInZone < 2)
                {
                    handle_PartySetSyncTarget(ipp, ipc::PartySetSyncTarget{ .partyId = party.getPartyId(), .charId = 0, .reason = MsgStd::LevelSyncRemoveTooFewMembers });
                }
            }

            if (message.destinationZoneId == 0xFFFF)
            {
                // Character is shutting down / logging out
                return handle_PartyRemoveMember(ipp, ipc::PartyRemoveMember{ .partyId = party.getPartyId(), .charId = message.charId });
            }

            // May need to reenable if we get odd behavior in the timeframe between a char zoning out and zoning in.
            return modifyParty(party.getPartyId(), &WorldParty::setMemberZone, message.charId, message.destinationZoneId);
            // return true;
        }

        ShowWarningFmt("CharZone message for charId {} but no party found", message.charId);
        return false;
    }

    bool handle_CharZoneIn(const IPP& ipp, const ipc::CharZoneIn& message)
    {
        // TODO: Check if the IPP is new and force a full update if so.
        // Find any party with the character
        // TODO: Could use a reverse lookup map or the character cache
        // clang-format off
        const auto it = std::find_if(m_Parties.begin(), m_Parties.end(), [&](auto& entry)
        {
            auto& party  = entry.second;
            auto members = party.getMembers();
            return std::any_of(members.begin(), members.end(), [&](auto& member)
            {
                return member.getId() == message.charId;
            });
        });
        // clang-format on

        if (it != m_Parties.end())
        {
            const auto& party = it->second;

            // Need to figure out what happens in the gap between zoning out and zoning in if we don't set it earlier
            return modifyParty(party.getPartyId(), &WorldParty::setMemberZone, message.charId, message.zoneId);
        }

        ShowWarningFmt("CharZoneIn message for charId {} but no party found", message.charId);
        return false;
    }

    bool handle_PartyCreate(const IPP& ipp, const ipc::PartyCreate& message)
    {
        auto [it, inserted] = this->m_Parties.emplace(message.charId, WorldParty(message.charId));
        ShowInfoFmt("Party created with charId: {}", message.charId);

        return handle_PartyAddMember(ipp, ipc::PartyAddMember{ .partyId = message.charId, .charId = message.charId, .type = PartyMemberType::Player });
    }

    bool handle_PartyAddMember(const IPP& ipp, const ipc::PartyAddMember& message)
    {
        uint16 zoneId = 0;
        if (message.type == PartyMemberType::Player)
        {
            const auto memberInfo = getCharInfoFromId(message.charId);
            if (!memberInfo)
            {
                ShowErrorFmt("Unable to find target member with ID: {}", message.charId);
                return false;
            }

            zoneId = memberInfo->zoneId;
        }
        else if (message.type == PartyMemberType::Trust)
        {
            if (const auto it = m_Parties.find(message.partyId); it != m_Parties.end())
            {
                zoneId = it->second.getLeader().value().get().getZone();
            }
            else
            {
                ShowErrorFmt("Tried to add a trust but could not find leader.");
                return false;
            }
        }

        const bool res = modifyParty(message.partyId, &WorldParty::addMember, message.charId, message.type, zoneId);

        if (!res && message.type == PartyMemberType::Player)
        {
            // Send Cannot be process to the invited player if we could not add them
            m_WorldServer.ipcServer_->rerouteMessageToCharId(
                message.charId,
                ipc::MessageBasic{
                    .recipientId = message.charId,
                    .message     = MsgStd::CannotBeProcessed,
                });
        }

        return res;
    }

    bool handle_PartyRemoveMember(const IPP& ipp, const ipc::PartyRemoveMember& message)
    {
        const auto it = m_Parties.find(message.partyId);
        if (it == m_Parties.end())
        {
            ShowErrorFmt("Party with ID {} not found", message.partyId);
            return false;
        }
        const auto& party        = it->second;
        const auto  syncTargetId = party.getSyncTarget() ? party.getSyncTarget().value().get().getId() : 0;
        const auto  leaderId     = party.getLeader().value().get().getId();
        uint32      newLeaderId  = 0;

        if (message.charId == leaderId)
        {
            // Leader is leaving, attempt to reassign lead
            const bool assignedNewLeader = modifyParty(message.partyId, &WorldParty::reassignLeader);
            if (!assignedNewLeader)
            {
                // Could not find any elegible leader / was last member.
                m_WorldServer.ipcServer_->rerouteMessageToCharId(message.charId, ipc::PlayerKick{ .victimId = message.charId });
                return handle_PartyDisband(ipp, ipc::PartyDisband{ .partyId = message.partyId });
            }
            newLeaderId = party.getLeader().value().get().getId();
            if (auto partyEntry = this->m_Parties.extract(leaderId); !partyEntry.empty())
            {
                // Insert it back under the new leader ID
                partyEntry.key() = newLeaderId;
                this->m_Parties.insert(std::move(partyEntry));
                ShowInfoFmt("Party leader set with partyId: {}", newLeaderId);
                // Tell map servers to update the partyId. This must happen before the next update.
                m_WorldServer.ipcServer_->rerouteMessageToPartyMembers(newLeaderId, ipc::PartyChangeId{ .formerId = leaderId, .newId = newLeaderId });
            }
        }

        const bool res = modifyParty(message.partyId, &WorldParty::removeMember, message.charId);
        if (res)
        {
            // Notify the player they've been kicked.
            m_WorldServer.ipcServer_->rerouteMessageToCharId(message.charId, ipc::PlayerKick{ .victimId = message.charId });

            // If we just removed the sync, we need to notify the party members
            if (syncTargetId == message.charId)
            {
                handle_PartySetSyncTarget(ipp, ipc::PartySetSyncTarget{ .partyId = party.getPartyId(), .charId = 0, .reason = MsgStd::LevelSyncRemoveLeftParty });
            }
        }

        return res;
    }

    bool handle_PartyDisband(const IPP& ipp, const ipc::PartyDisband& message)
    {
        if (const auto it = m_Parties.find(message.partyId); it != m_Parties.end())
        {
            // If leader requested breaking the PT, but we still have members, process them first.
            if (const auto members = it->second.getMembers(); members.size() != 0)
            {
                for (auto& member : members)
                {
                    handle_PartyRemoveMember(ipp, ipc::PartyRemoveMember{ .partyId = message.partyId, .charId = member.getId() });
                }
            }
        }

        this->m_Parties.erase(message.partyId);
        ShowInfoFmt("Party disbanded with partyId: {}", message.partyId);

        // Notify map servers that the party should no longer be tracked
        // TODO: Store IPPs we've interacted with and only send to those.
        m_WorldServer.ipcServer_->broadcastMessage(message);
        return true;
    }

    bool handle_PartySetLeader(const IPP& ipp, const ipc::PartySetLeader& message)
    {
        const auto it = m_Parties.find(message.partyId);
        if (it == m_Parties.end())
        {
            ShowErrorFmt("Party with ID {} not found", message.partyId);
            return false;
        }

        auto&      party       = it->second;
        const auto oldLeaderId = party.getLeader().value().get().getId();
        uint32     newLeaderId = message.charId;

        // Leader packets are usually sent with names rather than IDs.
        if (message.charId == 0 && !message.charName.empty())
        {
            const auto newLeaderInfo = getCharInfoFromName(message.charName);
            if (!newLeaderInfo)
            {
                ShowErrorFmt("Unable to find target leader with name: {}", message.charName);
                return false;
            }

            newLeaderId = newLeaderInfo->charId;
        }

        if (party.setLeader(newLeaderId))
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
            m_WorldServer.ipcServer_->rerouteMessageToPartyMembers(newLeaderId, ipc::PartyChangeId{ .formerId = oldLeaderId, .newId = newLeaderId });

            // Broadcast an update to force the map servers to push packets to the clients
            broadcastPartyUpdate(party);
            return true;
        }

        return false;
    }

    bool handle_PartySetQuartermaster(const IPP& ipp, const ipc::PartySetQuartermaster& message)
    {
        uint32 newQmId = message.charId;

        // QM packets are usually sent with names rather than IDs.
        if (newQmId == 0 && !message.charName.empty())
        {
            const auto newQmInfo = getCharInfoFromName(message.charName);

            if (!newQmInfo)
            {
                ShowErrorFmt("Unable to find target QM with name: {}", message.charName);
                return false;
            }

            newQmId = newQmInfo->charId;
            if (newQmId == 0)
            {
                ShowErrorFmt("Unable to find target QM with name: {}", message.charName);
                return false;
            }
        }

        return modifyParty(message.partyId, &WorldParty::setQuartermaster, newQmId);
    }

    bool handle_PartySetSyncTarget(const IPP& ipp, const ipc::PartySetSyncTarget& message)
    {
        bool immediateDisable = false;

        const auto it = m_Parties.find(message.partyId);
        if (it == m_Parties.end())
        {
            ShowErrorFmt("Party with ID {} not found", message.partyId);
            return false;
        }

        const auto&                       party        = it->second;
        const auto                        oldSync      = party.getSyncTarget();
        uint32                            syncTargetId = message.charId;
        std::unique_ptr<CharDatabaseData> syncTargetInfo;

        // Parameters sanity checks. We accept either ID or name but one must be set.
        if (syncTargetId == 0 && !message.charName.empty())
        {
            // Case 1. Syncing by name
            syncTargetInfo = getCharInfoFromName(message.charName);
            if (!syncTargetInfo)
            {
                ShowErrorFmt("Unable to find target sync target: {}", message.charName);
                return false;
            }
        }
        else if (syncTargetId != 0)
        {
            // Case 1. Syncing by ID
            syncTargetInfo = getCharInfoFromId(syncTargetId);
            if (!syncTargetInfo)
            {
                ShowErrorFmt("Unable to find target sync target with ID: {}", syncTargetId);
                return false;
            }
        }

        // We are about to apply sync
        if (syncTargetInfo)
        {
            syncTargetId = syncTargetInfo->charId;

            // Level sync rules enforcement
            // 1. Sync target must be in the same zone as the party leader
            if (syncTargetInfo->zoneId != party.getLeader().value().get().getZone())
            {
                m_WorldServer.ipcServer_->rerouteMessageToCharId(
                    party.getLeader().value().get().getId(),
                    ipc::MessageBasic{
                        .recipientId = party.getLeader().value().get().getId(),
                        .message     = MsgStd::LevelSyncDesigneeInOtherArea,
                    });

                return false;
            }

            // 2. Sync target must be above level 10
            if (syncTargetInfo->mLvl < 10)
            {
                m_WorldServer.ipcServer_->rerouteMessageToCharId(
                    party.getLeader().value().get().getId(),
                    ipc::MessageBasic{
                        .recipientId = party.getLeader().value().get().getId(),
                        .message     = MsgStd::LevelSyncDesigneeBelowMin,
                        .param1      = 10,
                    });

                return false;
            }

            // 3. Certain status effects block sync
            // TODO: This is super hard to check from this side, perhaps it should be handled when the map server receives the request from the leader
            // for (auto& member : members)
            // {
            //     if (member->StatusEffectContainer->HasStatusEffect({ EFFECT_LEVEL_RESTRICTION, EFFECT_LEVEL_SYNC, EFFECT_SJ_RESTRICTION, EFFECT_CONFRONTATION, EFFECT_BATTLEFIELD }))
            //     {
            //         ((CCharEntity*)getLeader())->pushPacket<CMessageBasicPacket>((CCharEntity*)getLeader(), (CCharEntity*)getLeader(), 0, 0, MsgStd::LevelSyncPreventedByStatus);
            //         return;
            //     }
            // }

            // Final check: If the target is alone in the zone, the sync is IMMEDIATELY disabled.
            // Verified to be retail accurate.
            uint8 membersInZone = 0;
            party.ForEveryMember({ .zoneId = syncTargetInfo->zoneId }, [&](const PartyMember& partyMember)
                                 { membersInZone++; });

            if (membersInZone < 2)
            {
                immediateDisable = true;
            }
        }

        const bool res = modifyParty(message.partyId, &WorldParty::setSyncTarget, syncTargetId);

        // If a reason was provided for disabling, we need to stream it to certain party members
        if (oldSync && syncTargetId == 0 && static_cast<uint16>(message.reason) != 0)
        {
            // clang-format off
            party.ForEveryMember({ .zoneId = oldSync.value().get().getZone() }, [&](const PartyMember& member)
            {
                m_WorldServer.ipcServer_->rerouteMessageToCharId(member.getId(),
                                                                 ipc::MessageBasic{
                                                                    .recipientId = member.getId(),
                                                                    .message     = message.reason,
                                                                    .param0      = 30,
                                                                    .param1     = 30,
                                                                 });
            });
            // clang-format on
        }

        if (immediateDisable)
        {
            return handle_PartySetSyncTarget(ipp, ipc::PartySetSyncTarget{ .partyId = message.partyId, .charId = 0, .reason = MsgStd::LevelSyncRemoveTooFewMembers });
        }

        return res;
    }

    bool handle_PartyUpdate(const IPP& ipp, const ipc::PartyUpdate& message)
    {
        m_Parties.emplace(message.partyId, WorldParty(message));

        return true;
    }

private:
    WorldServer&                           m_WorldServer;
    std::unordered_map<uint32, WorldParty> m_Parties;

    struct CharDatabaseData
    {
        uint32 charId;
        uint16 zoneId;
        uint8  mJob;
        uint8  mLvl;
        uint8  sJob;
        uint8  sLvl;
    };

    auto getCharInfoFromName(const std::string& name) const -> std::unique_ptr<CharDatabaseData>
    {
        const auto rset = db::preparedStmt("SELECT charid FROM chars WHERE charname = ? LIMIT 1", name);
        FOR_DB_SINGLE_RESULT(rset)
        {
            if (const auto charId = rset->get<uint32>("charid"); charId != 0)
            {
                return getCharInfoFromId(charId);
            }
        }

        ShowErrorFmt("Unable to find target with name: {}", name);
        return nullptr;
    }

    auto getCharInfoFromId(uint32 charId) const -> std::unique_ptr<CharDatabaseData>
    {
        TracyZoneScoped;

        const auto rset = db::preparedStmt(
            "SELECT cs.mjob, cs.mlvl, cs.sjob, cs.slvl, c.pos_zone "
            "FROM char_stats cs "
            "JOIN chars c ON cs.charid = c.charid "
            "WHERE cs.charid = ? "
            "LIMIT 1",
            charId);

        FOR_DB_SINGLE_RESULT(rset)
        {
            auto cdb = std::make_unique<CharDatabaseData>();

            cdb->mJob   = rset->get<uint8>("mjob");
            cdb->mLvl   = rset->get<uint8>("mlvl");
            cdb->sJob   = rset->get<uint8>("sjob");
            cdb->sLvl   = rset->get<uint8>("slvl");
            cdb->zoneId = rset->get<uint16>("pos_zone");
            cdb->charId = charId;

            return cdb;
        }

        return nullptr;
    }
};
