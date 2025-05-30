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

// TODO: Figure out a better name? This is more of a lightweight container than a system.
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

// Temporary debug function. To be extracted as a world server on-demand command later.
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

bool PartySystem::onCharZoneOut(const IPP& ipp, const ipc::CharZoneOut& message)
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
        auto& party = it->second;
        return party.setMemberZone(message.charId, message.destinationZoneId);
    }

    ShowWarningFmt("CharZone message for charId {} but no party found", message.charId);
    return false;
}

bool PartySystem::onCharZoneIn(const IPP& ipp, const ipc::CharZoneIn& message)
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
        auto& party = it->second;
        party.setMemberZone(message.charId, message.zoneId);
        // force update the new IPP so the new server knows about the party
        // TODO: Keep better track of IPPs so we can send only if this is a new one.
        // TODO: would rather handle it outside of this class
        notifyIppForParty(party.getPartyId(), party.asIpcUpdate());
        return true;
    }

    ShowWarningFmt("CharZoneIn message for charId {} but no party found", message.charId);
    return false;
}

bool PartySystem::createParty(uint32 leader)
{
    auto [it, inserted] = this->m_Parties.emplace(leader, WorldParty(leader));
    return inserted;
}

bool PartySystem::removeParty(uint32 partyId)
{
    if (const auto it = m_Parties.find(partyId); it != m_Parties.end())
    {
        m_Parties.erase(partyId);
        return true;
    }

    ShowErrorFmt("Party {} not found", partyId);
    return false;
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