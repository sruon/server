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

#include "npc_trade.h"

#include "items/item.h"
#include "items/item_store.h"

#include "common/mmo.h"

#include "entities/baseentity.h"
#include "entities/charentity.h"
#include "item_container.h"
#include "packets/s2c/0x01d_item_same.h"
#include "utils/charutils.h"

NpcTradeTransaction::NpcTradeTransaction(xi::Badge<NpcTradeTransaction>, CCharEntity* player, CBaseEntity* npc)
: Transaction()
, player_(player)
, npc_(npc)
{
}

auto NpcTradeTransaction::start(CCharEntity* player, CBaseEntity* npc) -> NpcTradeTransaction*
{
    if (player == nullptr || npc == nullptr)
    {
        return {};
    }

    if (player->activeTransaction<NpcTradeTransaction>() != nullptr)
    {
        // One NPC trade at a time. Client packet flow enforces this,
        // but defend against a replay.
        return {};
    }
    // Cross-type coexistence is fine (trading during a synth is
    // retail-accurate — 0x036's post-OnTrade path calls
    // forceSynthCritFail when isInEvent fires, which rolls the
    // synth back). The dup guard is ItemStore::moveToTransaction: an item
    // already InTransaction under another tx refuses re-stamping.

    auto tx = std::make_unique<NpcTradeTransaction>(xi::Badge<NpcTradeTransaction>{}, player, npc);
    return player->addTransaction(std::move(tx));
}

auto NpcTradeTransaction::takeSlot(std::uint8_t txSlot, std::uint8_t invSlot, std::uint32_t qty) -> bool
{
    if (txSlot >= kMaxSlots || qty == 0 || player_ == nullptr)
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
    if (item == nullptr || qty > item->getQuantity())
    {
        return false;
    }

    auto prior = ItemStore::moveToTransaction(item, this);
    if (!prior)
    {
        return false;
    }
    slots_[txSlot] = Slot{ item, invSlot, qty, 0, std::move(*prior) };
    return true;
}

auto NpcTradeTransaction::holds(const CItem* item) const -> bool
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

auto NpcTradeTransaction::doRollback() -> void
{
    for (auto& slot : slots_)
    {
        if (slot.item != nullptr)
        {
            restoreEntry(slot.item, slot.priorOwner);
            slot.item = nullptr;
        }
    }
}

auto NpcTradeTransaction::reserveFromOffer(std::uint16_t itemId, std::uint32_t qty) -> bool
{
    for (auto& slot : slots_)
    {
        if (slot.item != nullptr && slot.item->getID() == itemId &&
            slot.offeredQty >= qty && slot.reservedQty == 0)
        {
            slot.reservedQty = qty;
            return true;
        }
    }
    return false;
}

auto NpcTradeTransaction::clearReservation(std::uint8_t txSlot) -> void
{
    if (txSlot < kMaxSlots)
    {
        slots_[txSlot].reservedQty = 0;
    }
}

auto NpcTradeTransaction::offerGil(std::uint32_t amount) -> bool
{
    if (amount == 0 || player_ == nullptr)
    {
        return false;
    }
    auto* container = player_->getStorage(LOC_INVENTORY);
    if (container == nullptr)
    {
        return false;
    }
    auto* gilItem = container->GetItem(0);
    if (gilItem == nullptr || !gilItem->isType(ITEM_CURRENCY) ||
        gilItem->getQuantity() < amount)
    {
        return false;
    }
    offeredGil_ = amount;
    return true;
}

auto NpcTradeTransaction::reserveGil(std::uint32_t amount) -> bool
{
    if (amount > offeredGil_)
    {
        return false;
    }
    reservedGil_ = amount;
    return true;
}

auto NpcTradeTransaction::clearGilReservation() -> void
{
    reservedGil_ = 0;
}

auto NpcTradeTransaction::offeredGil() const -> std::uint32_t
{
    return isOpen() ? offeredGil_ : 0u;
}

auto NpcTradeTransaction::reservedGil() const -> std::uint32_t
{
    return isOpen() ? reservedGil_ : 0u;
}

auto NpcTradeTransaction::itemAt(std::uint8_t txSlot) const -> CItem*
{
    return (isOpen() && txSlot < kMaxSlots) ? slots_[txSlot].item : nullptr;
}

auto NpcTradeTransaction::offeredQtyAt(std::uint8_t txSlot) const -> std::uint32_t
{
    return isOpen() && txSlot < kMaxSlots ? slots_[txSlot].offeredQty : 0u;
}

auto NpcTradeTransaction::reservedQtyAt(std::uint8_t txSlot) const -> std::uint32_t
{
    return isOpen() && txSlot < kMaxSlots ? slots_[txSlot].reservedQty : 0u;
}

auto NpcTradeTransaction::commitAndClose() -> bool
{
    const bool ok = commit();
    player_->pushPacket<GP_SERV_COMMAND_ITEM_SAME>(player_);
    player_->removeTransaction(this); // destroys *this
    return ok;
}

auto NpcTradeTransaction::doCommit() -> bool
{
    if (player_ == nullptr)
    {
        return false;
    }

    // deliverToInventory must precede consume: ItemStore::consume operates
    // on InCharContainer items only; the tx's items are InTransaction.
    for (auto& slot : slots_)
    {
        if (slot.item == nullptr)
        {
            continue;
        }
        ItemStore::deliverToInventory(slot.item, player_->id, LOC_INVENTORY, slot.invSlot);
    }

    for (auto& slot : slots_)
    {
        if (slot.item == nullptr || slot.reservedQty == 0)
        {
            continue;
        }
        ItemStore::consume(player_, LOC_INVENTORY, slot.invSlot, slot.reservedQty);
        // consume drops the CItem if the stack hit zero; null the
        // slot before the dtor or any accessor can observe a stale
        // pointer.
        slot.item = nullptr;
    }

    if (reservedGil_ != 0)
    {
        charutils::UpdateItem(player_, LOC_INVENTORY, 0, -static_cast<std::int32_t>(reservedGil_));
    }
    return true;
}

auto NpcTradeTransaction::armPending(uint32_t eventId, CBaseEntity* npc, sol::protected_function onFinish) -> void
{
    pendingEventId_  = eventId;
    pendingNpc_      = npc;
    pendingOnFinish_ = std::move(onFinish);
}

auto NpcTradeTransaction::pendingEventId() const -> uint32_t
{
    return pendingEventId_;
}

auto NpcTradeTransaction::pendingNpc() const -> CBaseEntity*
{
    return pendingNpc_;
}

auto NpcTradeTransaction::pendingOnFinish() const -> const sol::protected_function&
{
    return pendingOnFinish_;
}
