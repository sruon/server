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

#include "party/system.h"
#include "ipc_server.h"
#include "party/world.h"

PartySystem::PartySystem(WorldServer& worldServer)
: m_WorldServer(worldServer)
{
}

WorldParty* PartySystem::getParty(const uint32 partyId)
{
    const auto it = m_Parties.find(partyId);
    return it != m_Parties.end() ? &it->second : nullptr;
}

void PartySystem::notifyIppForParty(const uint32 partyId, const auto& message, const uint16 zoneId) const
{
    if (const auto it = m_Parties.find(partyId); it != m_Parties.end())
    {
        for (const PartyMember& member : it->second.getMembers())
        {
            if (member.getZone() == zoneId)
            {
                m_WorldServer.ipcServer_->rerouteMessageToCharId(member.getId(), message);
            }
        }
    }
}

void PartySystem::notifyIppForParty(const uint32 partyId, const auto& message) const
{
    m_WorldServer.ipcServer_->rerouteMessageToPartyMembers(partyId, message);
}

void PartySystem::broadcastPartyUpdate(WorldParty& party) const
{
    if (party.isDirty())
    {
        for (auto& wrappedMember : party.getMembers())
        {
            PartyMember& member = wrappedMember;

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

void PartySystem::dump()
{
    for (auto& [partyId, party] : m_Parties)
    {
        ShowInfoFmt("Party {}: {}/6", partyId, party.getMemberCount());
        for (const PartyMember& member : party.getMembers())
        {
            std::string memberFlags = "";
            if (member.getType() == PartyMemberType::Player)
            {
                memberFlags = "Player";
            }
            else if (member.getType() == PartyMemberType::Trust)
            {
                memberFlags = "Trust";
            }
            if (member.getId() == party.getLeaderId())
            {
                memberFlags += ",Leader";
            }
            if (member.getId() == party.getQuartermasterId())
            {
                memberFlags += ",Quartermaster";
            }
            if (member.getId() == party.getSyncTargetId())
            {
                memberFlags += ",SyncTarget";
            }

            ShowInfoFmt("  Member: {} (joined {} ago) ({})", member.getId(), member.getTimeSinceJoined(), memberFlags);
        }
    }
}

bool PartySystem::handle_CharZoneOut(const IPP& ipp, const ipc::CharZoneOut& message)
{
    // Find any party with the character
    // TODO: Could use a reverse lookup map or the character cache
    // clang-format off
        const auto it = std::ranges::find_if(m_Parties, [&](auto& entry)
        {
            auto& party  = entry.second;
            auto members = party.getMembers();
            return std::any_of(members.begin(), members.end(), [&](const PartyMember& member)
            {
                return member.getId() == message.charId;
            });
        });
    // clang-format on

    if (it != m_Parties.end())
    {
        const auto& party = it->second;
        if (const auto syncTarget = party.getSyncTarget())
        {
            if (const PartyMember& target = syncTarget.value(); target.getId() == message.charId)
            {
                ShowInfoFmt("Sync target is zoning out. Removing sync target");
                handle_PartySetSyncTarget(ipp, ipc::PartySetSyncTarget{ .partyId = party.getPartyId(), .charId = 0, .reason = MsgStd::LevelSyncDeactivateLeftArea });
            }
            else
            {
                // Someone is zoning out, check if enough members are left in the sync zone.
                if (party.getMembers({ .zoneId = target.getZone() }).size() < 2)
                {
                    // Not enough players left, disable sync
                    handle_PartySetSyncTarget(ipp, ipc::PartySetSyncTarget{ .partyId = party.getPartyId(), .charId = 0, .reason = MsgStd::LevelSyncRemoveTooFewMembers });
                }
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

bool PartySystem::handle_CharZoneIn(const IPP& ipp, const ipc::CharZoneIn& message)
{
    // TODO: Check if the IPP is new and force a full update if so.
    // Find any party with the character
    // TODO: Could use a reverse lookup map or the character cache
    // clang-format off
        const auto it = std::ranges::find_if(m_Parties, [&](auto& entry)
        {
            auto& party  = entry.second;
            auto members = party.getMembers();
            return std::any_of(members.begin(), members.end(), [&](const PartyMember& member)
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

bool PartySystem::handle_PartyAddMember(const IPP& ipp, const ipc::PartyAddMember& message)
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
            if (const auto maybeLeader = it->second.getLeader())
            {
                const PartyMember& PLeader = maybeLeader.value();
                zoneId                     = PLeader.getZone();
            }
        }
        else
        {
            ShowErrorFmt("Tried to add a trust but could not find leader.");
            return false;
        }
    }

    // If the party does not exist, create it with the charId as the leader.
    if (getParty(message.partyId) == nullptr)
    {
        const auto leaderInfo = getCharInfoFromId(message.partyId);
        if (!leaderInfo)
        {
            ShowErrorFmt("Unable to find target leader with ID: {}", message.partyId);
            return false;
        }

        auto [it, inserted] = this->m_Parties.emplace(message.partyId, WorldParty(message.partyId));
        modifyParty(message.partyId, &WorldParty::addMember, message.partyId, message.type, leaderInfo->zoneId);
        ShowInfoFmt("Party created: {}", message.partyId);
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

bool PartySystem::handle_PartyRemoveMember(const IPP& ipp, const ipc::PartyRemoveMember& message)
{
    uint32 victimId = message.charId;

    const auto it = m_Parties.find(message.partyId);
    if (it == m_Parties.end())
    {
        ShowErrorFmt("Party with ID {} not found", message.partyId);
        return false;
    }

    const auto& party        = it->second;
    const auto  syncTargetId = party.getSyncTargetId();
    const auto  leaderId     = party.getLeaderId();

    if (victimId == 0 && !message.charName.empty())
    {
        const auto victimInfo = getCharInfoFromName(message.charName);
        if (!victimInfo)
        {
            ShowErrorFmt("Unable to find target member with name: {}", message.charName);
            return false;
        }
        victimId = victimInfo->charId;
    }

    const bool res = modifyParty(message.partyId, &WorldParty::removeMember, victimId);
    if (res)
    {
        // If we just removed the sync, we need to notify the party members
        if (syncTargetId == victimId)
        {
            handle_PartySetSyncTarget(ipp, ipc::PartySetSyncTarget{ .partyId = party.getPartyId(), .charId = 0, .reason = MsgStd::LevelSyncRemoveLeftParty });
        }
    }

    if (party.getMemberCount() == 0)
    {
        return handle_PartyDisband(ipp, ipc::PartyDisband{ .partyId = message.partyId });
    }

    // Some special handling to be done if leader left
    if (victimId == leaderId)
    {
        uint32 newLeaderId = party.getLeaderId();

        if (newLeaderId == 0)
        {
            // Could not find any eligible leader / was last member.
            return handle_PartyDisband(ipp, ipc::PartyDisband{ .partyId = message.partyId });
        }

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

    return res;
}

bool PartySystem::handle_PartyDisband(const IPP& ipp, const ipc::PartyDisband& message)
{
    if (const auto it = m_Parties.find(message.partyId); it != m_Parties.end())
    {
        // If leader requested breaking the PT, but we still have members, process them first.
        for (const auto& member : it->second.getMembers())
        {
                handle_PartyRemoveMember(ipp, ipc::PartyRemoveMember{ .partyId = message.partyId, .charId = member.get().getId() });
        }
    }

    this->m_Parties.erase(message.partyId);
    ShowInfoFmt("Party disbanded with partyId: {}", message.partyId);

    // Notify map servers that the party should no longer be tracked
    // TODO: Store IPPs we've interacted with and only send to those.
    m_WorldServer.ipcServer_->broadcastMessage(message);
    return true;
}

bool PartySystem::handle_PartySetLeader(const IPP& ipp, const ipc::PartySetLeader& message)
{
    const auto it = m_Parties.find(message.partyId);
    if (it == m_Parties.end())
    {
        ShowErrorFmt("Party with ID {} not found", message.partyId);
        return false;
    }

    auto&      party       = it->second;
    const auto oldLeaderId = party.getLeaderId();
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

bool PartySystem::handle_PartySetQuartermaster(const IPP& ipp, const ipc::PartySetQuartermaster& message)
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

// A map server has forwarded a request from a party leader to enable Level Sync on their party.
bool PartySystem::handle_PartySetSyncTarget(const IPP& ipp, const ipc::PartySetSyncTarget& message)
{
    bool immediateDisable = false;

    const auto it = m_Parties.find(message.partyId);
    if (it == m_Parties.end())
    {
        ShowErrorFmt("Party with ID {} not found", message.partyId);
        return false;
    }

    const auto&                       party        = it->second;
    const auto                        maybeOldSync = party.getSyncTarget();
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
        if (const auto maybeLeader = party.getLeader())
        {
            const PartyMember& leader = maybeLeader.value();

            // 1. Sync target must be in the same zone as the party leader
            if (syncTargetInfo->zoneId != leader.getZone())
            {
                m_WorldServer.ipcServer_->rerouteMessageToCharId(
                    leader.getId(),
                    ipc::MessageBasic{
                        .recipientId = leader.getId(),
                        .message     = MsgStd::LevelSyncDesigneeInOtherArea,
                    });

                return false;
            }

            // 2. Sync target must be above level 10
            if (syncTargetInfo->mLvl < 10)
            {
                m_WorldServer.ipcServer_->rerouteMessageToCharId(
                    leader.getId(),
                    ipc::MessageBasic{
                        .recipientId = leader.getId(),
                        .message     = MsgStd::LevelSyncDesigneeBelowMin,
                        .param1      = 10,
                    });

                return false;
            }

            // 3. Certain status effects block sync
            // TODO: Map server is authoritative for this check in SmallPacket0x077
            // Status effects are not saved to database unless specific actions are performed, so we are unable to get a proper view from this side.

            // 4. If the target is alone in the zone, the sync is IMMEDIATELY disabled.
            // Verified to be retail accurate.
            if (party.getMembers({ .zoneId = syncTargetInfo->zoneId }).size() < 2)
            {
                immediateDisable = true;
            }
        }
        else
        {
            // Unlikely we don't have a leader, but if we do, we can't apply sync.
            return false;
        }
    }

    const bool res = modifyParty(message.partyId, &WorldParty::setSyncTarget, syncTargetId);

    // If a reason was provided for disabling, we need to stream it to certain party members
    if (maybeOldSync && syncTargetId == 0 && static_cast<uint16>(message.reason) != 0)
    {
        const PartyMember& oldSync = maybeOldSync.value();

        // clang-format off
        party.ForEveryMember({ .zoneId = oldSync.getZone() }, [&](const PartyMember& member)
        {
            m_WorldServer.ipcServer_->rerouteMessageToCharId(member.getId(),
                ipc::MessageBasic{
                    .recipientId = member.getId(),
                    .message     = message.reason,
                    .param0      = 30,
                    .param1      = 30,
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

// Updates received from the map servers.
// This is used to recover after an eventual world server crash/restart
bool PartySystem::handle_PartyUpdate(const IPP& ipp, const ipc::PartyUpdate& message)
{
    // TODO: This is incredibly naive and may not work well with multiple map servers.
    // If this doesn't prove to work reliably, we could fallback to using the database instead.
    m_Parties.emplace(message.partyId, WorldParty(message));

    return true;
}