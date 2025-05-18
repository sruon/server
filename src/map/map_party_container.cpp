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
    auto it = parties_.find(message.partyId);
    if (it == parties_.end())
    {
        // Party doesn't exist, create a new one
        auto newParty             = std::make_unique<CCharParty>(message);
        parties_[message.partyId] = std::move(newParty);
    }
    else
    {
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