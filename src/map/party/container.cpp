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

#include "container.h"

#include "entities/charentity.h"
#include "utils/zoneutils.h"

void PartyContainer::updateParty(const ipc::PartyUpdate& message)
{
    if (const auto it = m_Parties.find(message.partyId); it == m_Parties.end())
    {
        ShowInfoFmt("Creating new party with ID: {}", message.partyId);
        // Party doesn't exist, create a new one
        auto newParty             = CCharParty::Create(message);
        m_Parties[message.partyId] = std::move(newParty);
    }
    else
    {
        ShowInfoFmt("Updating existing party with ID: {}", message.partyId);
        m_Parties[message.partyId]->update(message);
    }

    // Retail packet flow:
    // 0xC8: Defines party layout
    // 0xE2: Char Info with trust data
    // 0x0E: NPC update with trust
    // 0x67: Entity status
    // 0xDF: Char update with trust data
    // 0x0E: Several NPC updates with name etc
}

void PartyContainer::updateId(const uint32 old, const uint32 newId)
{
    ShowInfoFmt("Updating party ID from {} to {}", old, newId);
    if (const auto it = m_Parties.find(old); it != m_Parties.end())
    {
        auto party = std::move(it->second);
        party->setPartyId(newId);
        m_Parties[newId] = std::move(party);
        m_Parties.erase(it);
    }
}

void PartyContainer::chatMessage(const ipc::ChatMessageParty& message)
{
    if (const auto it = m_Parties.find(message.partyId); it != m_Parties.end())
    {
        it->second->ChatMessage(message);
    }
}

void PartyContainer::chatMessage(const ipc::ChatMessageAlliance& message)
{
    // TODO: this is wrong
    if (const auto it = m_Parties.find(message.allianceId); it != m_Parties.end())
    {
        it->second->ChatMessage(message);
    }
}

void PartyContainer::disbandParty(const ipc::PartyDisband& message)
{
    if (const auto it = m_Parties.find(message.partyId); it != m_Parties.end())
    {
        ShowInfoFmt("Removing party ID {} from party container", message.partyId);
        m_Parties.erase(it);
    }
}