//
// Created by sruon on 5/17/2025.
//

#include "map_party_container.h"

#include "entities/charentity.h"
#include "entities/trustentity.h"
#include "packets/party_define.h"
#include "packets/party_member_update.h"
#include "utils/zoneutils.h"

void PartyContainer::updateParty(const ipc::PartyUpdate& message)
{
    if (const auto it = parties_.find(message.partyId); it == parties_.end())
    {
        ShowInfoFmt("Creating new party with ID: {}", message.partyId);
        // Party doesn't exist, create a new one
        auto newParty             = CCharParty::Create(message);
        parties_[message.partyId] = std::move(newParty);
    }
    else
    {
        ShowInfoFmt("Updating existing party with ID: {}", message.partyId);
        parties_[message.partyId]->update(message);
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
    if (const auto it = parties_.find(old); it != parties_.end())
    {
        auto party = std::move(it->second);
        party->setPartyId(newId);
        parties_[newId] = std::move(party);
        parties_.erase(it);
    }
}

void PartyContainer::chatMessage(const ipc::ChatMessageParty& message)
{
    if (const auto it = parties_.find(message.partyId); it != parties_.end())
    {
        it->second->ChatMessage(message);
    }
}

void PartyContainer::chatMessage(const ipc::ChatMessageAlliance& message)
{
    // TODO: this is wrong
    if (const auto it = parties_.find(message.allianceId); it != parties_.end())
    {
        it->second->ChatMessage(message);
    }
}