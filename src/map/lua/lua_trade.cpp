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

#include "lua_trade.h"

#include "items/item.h"
#include "items/transactions/npc_trade.h"

#include "entities/charentity.h"
#include "luautils.h"

#include <sol/sol.hpp>

namespace
{
constexpr uint16 kGilItemId = 0xFFFF;

// Count of non-null tx slots. The 0x036 handler packs items densely
// (gil is routed to offeredGil_, doesn't consume a tx slot), so
// itemCount also equals the index of the virtual gil slot when gil
// is offered.
auto itemCount(const NpcTradeTransaction* t) -> uint8
{
    uint8 count = 0;
    for (uint8 slot = 0; slot < NpcTradeTransaction::kMaxSlots; ++slot)
    {
        if (t->itemAt(slot) != nullptr)
        {
            ++count;
        }
    }
    return count;
}
} // namespace

CLuaTrade::CLuaTrade(CCharEntity* player)
: player_(player)
{
}

auto CLuaTrade::tx() const -> NpcTradeTransaction*
{
    if (player_ == nullptr)
    {
        return nullptr;
    }
    return player_->activeTransaction<NpcTradeTransaction>();
}

auto CLuaTrade::getItem(const sol::object& slotObj) -> CItem*
{
    auto* t = tx();
    if (t == nullptr)
    {
        return nullptr;
    }
    const uint8 slot = slotObj.is<uint8>() ? slotObj.as<uint8>() : uint8{ 0 };
    if (slot >= NpcTradeTransaction::kMaxSlots)
    {
        return nullptr;
    }
    return t->itemAt(slot);
}

auto CLuaTrade::getItemId(const sol::object& slotObj) -> uint16
{
    auto* t = tx();
    if (t == nullptr)
    {
        return 0;
    }
    const uint8 slot = slotObj.is<uint8>() ? slotObj.as<uint8>() : uint8{ 0 };
    // Gil occupies the logical slot immediately after the last item.
    if (t->offeredGil() != 0 && slot == itemCount(t))
    {
        return kGilItemId;
    }
    if (slot >= NpcTradeTransaction::kMaxSlots)
    {
        return 0;
    }
    auto* item = t->itemAt(slot);
    return item ? item->getID() : 0;
}

auto CLuaTrade::getItemSubId(const sol::object& slotObj) -> uint16
{
    auto* item = getItem(slotObj);
    return item ? item->getSubID() : 0;
}

auto CLuaTrade::getItemQty(uint16 itemId) -> uint32
{
    auto* t = tx();
    if (t == nullptr)
    {
        return 0;
    }
    if (itemId == kGilItemId)
    {
        return t->offeredGil();
    }
    uint32 total = 0;
    for (uint8 slot = 0; slot < NpcTradeTransaction::kMaxSlots; ++slot)
    {
        auto* item = t->itemAt(slot);
        if (item != nullptr && item->getID() == itemId)
        {
            total += t->offeredQtyAt(slot);
        }
    }
    return total;
}

auto CLuaTrade::hasItemQty(uint16 itemId, uint32 quantity) -> bool
{
    return getItemQty(itemId) >= quantity;
}

auto CLuaTrade::getGil() -> uint32
{
    auto* t = tx();
    return t ? t->offeredGil() : 0;
}

auto CLuaTrade::getSlotQty(uint8 slot) -> uint32
{
    auto* t = tx();
    if (t == nullptr)
    {
        return 0;
    }
    if (t->offeredGil() != 0 && slot == itemCount(t))
    {
        return t->offeredGil();
    }
    return slot < NpcTradeTransaction::kMaxSlots ? t->offeredQtyAt(slot) : 0;
}

auto CLuaTrade::getSlotCount() -> uint8
{
    auto* t = tx();
    if (t == nullptr)
    {
        return 0;
    }
    // Legacy semantics: count of occupied slots, + 1 if gil offered.
    // Tx slots are packed densely at low indices by the 0x036 handler,
    // so getItemId(i) for i < count returns a real item; i == count-1
    // returns 0xFFFF when gil is present.
    uint8 count = 0;
    for (uint8 slot = 0; slot < NpcTradeTransaction::kMaxSlots; ++slot)
    {
        if (t->itemAt(slot) != nullptr)
        {
            ++count;
        }
    }
    if (t->offeredGil() != 0)
    {
        ++count;
    }
    return count;
}

auto CLuaTrade::getItemCount() -> uint32
{
    auto* t = tx();
    if (t == nullptr)
    {
        return 0;
    }
    uint32 total = 0;
    for (uint8 slot = 0; slot < NpcTradeTransaction::kMaxSlots; ++slot)
    {
        if (t->itemAt(slot) != nullptr)
        {
            total += t->offeredQtyAt(slot);
        }
    }
    return total;
}

auto CLuaTrade::confirmItem(uint16 itemId, const sol::object& amountObj) -> bool
{
    auto* t = tx();
    if (t == nullptr)
    {
        return false;
    }
    const uint32 amount = amountObj.is<uint32>() ? amountObj.as<uint32>() : uint32{ 1 };
    if (itemId == kGilItemId)
    {
        return t->reserveGil(amount);
    }
    return t->reserveFromOffer(itemId, amount);
}

auto CLuaTrade::confirmSlot(uint8 slot, const sol::object& amountObj) -> bool
{
    auto* t = tx();
    if (t == nullptr)
    {
        return false;
    }
    if (t->offeredGil() != 0 && slot == itemCount(t))
    {
        const uint32 amount = amountObj.is<uint32>() ? amountObj.as<uint32>() : t->offeredGil();
        return t->reserveGil(amount);
    }
    if (slot >= NpcTradeTransaction::kMaxSlots)
    {
        return false;
    }
    auto* item = t->itemAt(slot);
    if (item == nullptr)
    {
        return false;
    }
    const uint32 offered = t->offeredQtyAt(slot);
    const uint32 amount  = amountObj.is<uint32>() ? amountObj.as<uint32>() : offered;
    return t->reserveFromOffer(item->getID(), amount);
}

auto CLuaTrade::clean() -> void
{
    auto* t = tx();
    if (t == nullptr || player_ == nullptr)
    {
        return;
    }
    // Legacy "abandon the trade" semantic — items return to player via
    // the tx rollback path driven by removeTransaction.
    player_->removeTransaction(t);
}

void CLuaTrade::Register()
{
    SOL_USERTYPE("CTradeContainer", CLuaTrade);
    SOL_REGISTER("getItem", CLuaTrade::getItem);
    SOL_REGISTER("getItemId", CLuaTrade::getItemId);
    SOL_REGISTER("getItemSubId", CLuaTrade::getItemSubId);
    SOL_REGISTER("getItemQty", CLuaTrade::getItemQty);
    SOL_REGISTER("hasItemQty", CLuaTrade::hasItemQty);
    SOL_REGISTER("getGil", CLuaTrade::getGil);
    SOL_REGISTER("getSlotQty", CLuaTrade::getSlotQty);
    SOL_REGISTER("getSlotCount", CLuaTrade::getSlotCount);
    SOL_REGISTER("getItemCount", CLuaTrade::getItemCount);
    SOL_REGISTER("confirmItem", CLuaTrade::confirmItem);
    SOL_REGISTER("confirmSlot", CLuaTrade::confirmSlot);
    SOL_REGISTER("clean", CLuaTrade::clean);
}
