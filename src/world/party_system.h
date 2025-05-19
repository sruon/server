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
    uint32                                    m_PartyId = 0;
    std::array<std::optional<PartyMember>, 6> m_Members{};
    uint32                                    m_LeaderUniqueNo        = 0;
    uint32                                    m_QuarterMasterUniqueNo = 0;
    uint32                                    m_SyncTargetUniqueNo    = 0;
    bool                                      dirty                   = true;

public:
    Party(uint32 leaderId)
    : m_LeaderUniqueNo(leaderId)
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

    auto GetLeader() const -> const PartyMember*
    {
        // Lambda function to check if an optional party member is the leader
        const auto isLeader = [&](const std::optional<PartyMember>& opt_member) -> bool
        {
            // First check if the optional contains a value
            if (!opt_member.has_value())
            {
                return false;
            }

            // Then check if the contained member is the leader
            return opt_member.value().GetId() == m_LeaderUniqueNo;
        };

        // Find the first optional member that matches the isLeader condition
        const auto leader = std::find_if(m_Members.begin(), m_Members.end(), isLeader);

        // If found, return a pointer to the PartyMember, otherwise return nullptr
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
            // First check if the optional contains a value
            if (!opt_member.has_value())
            {
                return false;
            }

            // Then check if the contained member is the quartermaster
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
            // First check if the optional contains a value
            if (!opt_member.has_value())
            {
                return false;
            }

            // Then check if the contained member is the sync target
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

    bool AddMember(uint32_t UniqueNo, PartyMemberType type, uint32 ZoneId)
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
                memberSlot = PartyMember{ UniqueNo, type, ZoneId };
                ShowInfoFmt("Added member with UniqueNo: {} (type {})", UniqueNo, static_cast<uint8>(type));
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
                // Notify of sync wearing out

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

    template <typename Func, typename... Args>
    bool ModifyParty(uint16 partyId, Func&& func, Args&&... args)
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
            ShowInfoFmt("Notifying map servers that party {} is dirty.", party.GetPartyId());
            worldServer_.ipcServer_->rerouteMessageToPartyMembers(party.GetPartyId(), party.AsPartyUpdate());
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
        auto [it, inserted] = this->m_Parties.emplace(message.charId, Party(message.charId));
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
        const bool res = ModifyParty(message.partyId, &Party::RemoveMember, message.charId);
        if (res)
        {
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
        return ModifyParty(message.partyId, &Party::SetSyncTarget, message.charId);
    }

private:
    WorldServer&                      worldServer_;
    std::unordered_map<uint16, Party> m_Parties;
};
