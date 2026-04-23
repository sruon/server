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

#include "common/types/badge.h"
#include "items/item_owner.h"
#include "items/transaction.h"
#include "items/transaction_handle.h"

#include <array>
#include <cstdint>

class CBaseEntity;
class CCharEntity;
class CItem;

// NpcTradeTransaction — custody class for an NPC trade's offer phase.
//
// Opens when the client sends 0x036 item_transfer. Each offered
// inventory slot's CItem transitions InCharContainer → InTransaction via
// moveToTransaction. Custody persists across event callbacks: a trade that
// starts a cutscene/dialog keeps the tx alive through
// onEventUpdate/onEventFinish so rewards can be granted and the
// commit sequenced with the event's end.
//
// Commit semantics:
//   - Reserved slots: decrement consume-qty on the underlying stack
//     (destroys the CItem if qty hits 0). Unreserved qty stays with
//     the character.
//   - Unreserved slots: deliverToInventory (restore owner) — no decrement.
//
// Rollback semantics: all items deliverToInventory, no decrement.
//
// Partial-stack semantics: the tx takes custody of the entire CItem
// (whole stack). consume-qty on commit decrements the stack; if the
// stack is larger than the offered qty, the remainder stays in the
// player's inventory.

class NpcTradeTransaction : public Transaction
{
public:
    static constexpr std::size_t kMaxSlots = 8; // TRADE_CONTAINER_SIZE

    // Open the tx. Installs on `player`. Returns an empty TransactionHandle if
    // `player` already has a live NpcTradeTransaction (one-at-a-time per player).
    static auto start(CCharEntity* player, CBaseEntity* npc) -> TransactionHandle<NpcTradeTransaction>;

    // Move an inventory item into the tx at `txSlot`. Records the
    // consume-qty (can be < stack size). Returns false if the slot is
    // occupied, qty is invalid, or the item can't be taken.
    auto takeSlot(std::uint8_t txSlot, std::uint8_t invSlot, std::uint32_t qty) -> bool;

    // Record the gil amount offered in the trade. Packet slot 0 is
    // the gil slot in FFXI's trade protocol; 0x036 handler routes it
    // here instead of takeSlot. Unlike items, gil is not moved into
    // tx custody — it's pure intent, consumed (delGil) only if
    // reserved on commit. Returns false if the player doesn't have
    // at least `amount` gil at offer time.
    auto offerGil(std::uint32_t amount) -> bool;

    // Mark `amount` gil as reserved-for-consumption. Capped at
    // offeredGil. Returns true iff the requested amount is <= offered.
    auto reserveGil(std::uint32_t amount) -> bool;

    // Clear the gil reservation so a later decl can retry.
    auto clearGilReservation() -> void;

    // Find the first unreserved slot with item id `itemId` offering
    // at least `qty` and mark it reserved. Returns true on match.
    // Used by declarative-trade phase-1 matching. Nothing is consumed
    // until commitAndClose(); this only sets reservedQty.
    auto reserveFromOffer(std::uint16_t itemId, std::uint32_t qty) -> bool;

    // Clear a tx-slot's reservedQty back to 0. Used by the declarative
    // engine when a decl's match-phase fails partway: previous
    // reservations must be undone so the next decl can match against
    // the original offer.
    auto clearReservation(std::uint8_t txSlot) -> void;

    // Commit the tx and tear it down: pushes ITEM_SAME to the player
    // and removeTransaction. Destroys *this — must be last. Returns the
    // underlying commit() result.
    auto commitAndClose() -> bool;

    // Read-only accessors used by the declarative-trade runner. All
    // return defaults when the tx is no longer Open (subclass slots_
    // may hold stale pointers after a successful commit).
    auto itemAt(std::uint8_t txSlot) const -> CItem*;
    auto offeredQtyAt(std::uint8_t txSlot) const -> std::uint32_t;
    auto reservedQtyAt(std::uint8_t txSlot) const -> std::uint32_t;
    auto offeredGil() const -> std::uint32_t;
    auto reservedGil() const -> std::uint32_t;

    auto player() const -> CCharEntity*
    {
        return player_;
    }

    // Badge-gated public ctor so make_unique can reach it.
    NpcTradeTransaction(xi::Badge<NpcTradeTransaction>, CCharEntity* player, CBaseEntity* npc);

    ~NpcTradeTransaction() override
    {
        silentRollbackIfOpen();
    }

    auto holds(const CItem* item) const -> bool override;

protected:
    auto doCommit() -> bool override;
    auto doRollback() -> void override;

private:
    struct Slot
    {
        CItem*        item{};
        std::uint8_t  invSlot{ 0xFF };
        std::uint32_t offeredQty{};
        std::uint32_t reservedQty{};
        ItemOwner     priorOwner{};
    };

    CCharEntity*                player_{};
    CBaseEntity*                npc_{};
    std::array<Slot, kMaxSlots> slots_{};
    std::uint32_t               offeredGil_{};
    std::uint32_t               reservedGil_{};
};
