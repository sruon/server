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

#pragma once

#include "common/cbasetypes.h"

#include <sol/forward.hpp>

class CCharEntity;
class CItem;
class NpcTradeTransaction;

// CLuaTrade — legacy-compat read/reserve wrapper around the active
// NpcTradeTransaction. Legacy entity.onTrade handlers receive this
// as their `trade` argument via player:getTrade(), so existing code
// (npcUtil.tradeHasExactly, trade:getItemId(), etc.) continues to
// work on top of the new tx-based custody model.
//
// The wrapper stores a CCharEntity* and looks up the active tx on
// each call — safer than caching a raw tx pointer, which would
// dangle if confirmTrade / clean destroyed it mid-handler.
//
// Gil convention (retail-legacy): offered gil appears in iteration
// at slot `kMaxSlots` with item id 0xFFFF. `getItemQty(0xFFFF)`
// returns offered gil. `getSlotCount()` bumps by 1 when gil is
// offered so `for i = 0, getSlotCount()-1` iterates over gil too.

class CLuaTrade
{
public:
    explicit CLuaTrade(CCharEntity* player);

    // ---- Read API expected by legacy entity.onTrade scripts ----------

    auto getItem(const sol::object& slotObj) -> CItem*;
    auto getItemId(const sol::object& slotObj) -> uint16;
    auto getItemSubId(const sol::object& slotObj) -> uint16;

    // Sum of offered quantity for `itemId` across all slots. Returns
    // offered gil when itemId == 0xFFFF (retail-legacy marker).
    auto getItemQty(uint16 itemId) -> uint32;

    auto hasItemQty(uint16 itemId, uint32 quantity) -> bool;
    auto getGil() -> uint32;

    // Quantity offered in a specific slot. For the gil pseudo-slot
    // (index kMaxSlots) returns offered gil.
    auto getSlotQty(uint8 slot) -> uint32;

    // Iteration bound. kMaxSlots + 1 when gil is offered, kMaxSlots
    // otherwise.
    auto getSlotCount() -> uint8;

    // Total quantity across all non-empty slots (excludes gil).
    auto getItemCount() -> uint32;

    // ---- Reserve surface (maps to NpcTradeTransaction internals) ------

    // Reserve `amount` of `itemId` for consumption on commit. For
    // itemId == 0xFFFF, reserves gil. Returns false when the amount
    // isn't available in the offer.
    auto confirmItem(uint16 itemId, const sol::object& amountObj) -> bool;

    // Reserve `amount` from a specific slot (defaults to the slot's
    // full offered qty when amount is nil). Matches legacy
    // trade:confirmSlot(i) semantics used by synergy / guild points.
    auto confirmSlot(uint8 slot, const sol::object& amountObj) -> bool;

    // Roll back the active tx without consuming. Legacy escape hatch
    // (ZNM ryo, synergy facility). Items return to the player.
    auto clean() -> void;

    // ---- sol required ----

    bool operator==(const CLuaTrade& other) const
    {
        return player_ == other.player_;
    }

    static void Register();

private:
    auto tx() const -> NpcTradeTransaction*;

    CCharEntity* player_{};
};
