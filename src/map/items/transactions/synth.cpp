/*
===========================================================================

  Copyright (c) 2026 LandSandBoat Dev Teams

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

#include "synth.h"

#include "common/mmo.h"

#include "items/craft_state.h"
#include "items/item.h"
#include "items/item_store.h"

#include "entities/charentity.h"
#include "item_container.h"
#include "utils/charutils.h"

SynthTransaction::SynthTransaction(xi::Badge<SynthTransaction>, CCharEntity* player)
: Transaction()
, player_(player)
{
}

auto SynthTransaction::start(CCharEntity* player) -> SynthTransaction*
{
    if (player == nullptr)
    {
        return {};
    }
    if (player->activeTransaction<SynthTransaction>() != nullptr)
    {
        return {};
    }

    auto tx = std::make_unique<SynthTransaction>(xi::Badge<SynthTransaction>{}, player);
    return player->addTransaction(std::move(tx));
}

auto SynthTransaction::takeSlot(std::uint8_t txSlot, std::uint8_t invSlot) -> bool
{
    if (txSlot >= kMaxSlots || player_ == nullptr)
    {
        return false;
    }
    if (slots_[txSlot].item != nullptr)
    {
        return false;
    }

    auto* container = player_->getStorage(LOC_INVENTORY);
    if (container == nullptr)
    {
        return false;
    }
    auto* item = container->GetItem(invSlot);
    if (item == nullptr)
    {
        return false;
    }

    auto prior = ItemStore::moveToTransaction(item, this);
    if (!prior)
    {
        return false;
    }
    slots_[txSlot] = Slot{ item, invSlot, std::move(*prior) };
    return true;
}

auto SynthTransaction::holds(const CItem* item) const -> bool
{
    if (!isOpen() || item == nullptr)
    {
        return false;
    }
    for (const auto& slot : slots_)
    {
        if (slot.item == item)
        {
            return true;
        }
    }
    return false;
}

auto SynthTransaction::doRollback() -> void
{
    for (auto& slot : slots_)
    {
        if (slot.item != nullptr)
        {
            restoreEntry(slot.item, slot.priorOwner);
            slot.item    = nullptr;
            slot.invSlot = 0xFF;
        }
    }
}

auto SynthTransaction::takeFromCraftState(const CCraftState& state) -> void
{
    // Slot 0 = crystal.
    if (state.crystalInvSlot != 0xFF)
    {
        takeSlot(0, state.crystalInvSlot);
    }
    // Slots 1-8 = ingredients (craftState.ingredients is 0-indexed).
    for (std::size_t i = 0; i < state.ingredients.size(); ++i)
    {
        const auto invSlot = state.ingredients[i].invSlot;
        if (invSlot != 0xFF)
        {
            takeSlot(static_cast<std::uint8_t>(i + 1), invSlot);
        }
    }
}

auto SynthTransaction::consumeCrystal() -> void
{
    static constexpr std::uint8_t kCrystalSlot = 0;
    auto&                         slot         = slots_[kCrystalSlot];
    if (slot.item == nullptr)
    {
        return;
    }
    const std::uint8_t invSlot = slot.invSlot;
    releaseSlot(kCrystalSlot);
    charutils::UpdateItem(player_, LOC_INVENTORY, invSlot, -1);
}

auto SynthTransaction::releaseSlot(std::uint8_t txSlot) -> void
{
    if (txSlot >= kMaxSlots || player_ == nullptr)
    {
        return;
    }
    auto& slot = slots_[txSlot];
    if (slot.item == nullptr)
    {
        return;
    }
    ItemStore::deliverToInventory(slot.item, player_->id, LOC_INVENTORY, slot.invSlot);
    slot.item    = nullptr;
    slot.invSlot = 0xFF;
}

auto SynthTransaction::doCommit() -> bool
{
    if (player_ == nullptr)
    {
        return false;
    }
    for (auto& slot : slots_)
    {
        if (slot.item == nullptr)
        {
            continue;
        }
        ItemStore::deliverToInventory(slot.item, player_->id, LOC_INVENTORY, slot.invSlot);
        // Null the slot — synthutils will drive UpdateItem next; any
        // zero-qty destroy leaves a dangling pointer we must not touch.
        slot.item    = nullptr;
        slot.invSlot = 0xFF;
    }
    return true;
}
