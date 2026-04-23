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

class CCharEntity;
class CItem;

// PlayerTradeTransaction — single tx representing a player-to-player trade.
//
// Owned exclusively by the initiator via `initiator->transactions`
// (std::unique_ptr). The partner reaches the tx through the existing
// tradePending → partner entity lookup (CCharEntity::activeTradeTx).
//
// Custody-at-offer: each side's offered items transition InCharContainer
// → InTransaction at 0x034 trade_list time via takeSlot(). Slot
// replacement (dropping a slot or changing its qty) returns the prior
// CItem to InCharContainer before the new one enters custody.
//
// Commit semantics: atomic two-sided swap. doCommit pre-validates
// both inventories can receive, then hands each side's items to the
// other via ItemStore::deliverToInventory → ItemStore::consume on the
// source, charutils::AddItem on the destination. If either half
// would fail (inventory full, rare-dup), both halves roll back.
//
// Rollback semantics: every held item returns to its original
// inventory slot via the base class restoreFromTransaction path.

class PlayerTradeTransaction : public Transaction
{
public:
    static constexpr std::size_t kMaxSlots = 9; // UCONTAINER_SIZE (trade side)

    // Open the trade. Installs exclusively on `initiator`. Returns an
    // empty TransactionHandle if either side already has an active PlayerTradeTransaction.
    static auto start(CCharEntity* initiator, CCharEntity* partner) -> TransactionHandle<PlayerTradeTransaction>;

    auto initiator() const -> CCharEntity*;
    auto partner() const -> CCharEntity*;

    // One side is leaving — abort the trade on their behalf. Notifies
    // the counterparty's client (wake them out of the trade UI), clears
    // tradePending on both sides, and removes the tx from the
    // initiator. Callable from either side. Destroys `this` via
    // removeTransaction — do not touch the tx after calling.
    auto abort(CCharEntity* leaving) -> void;

    // Accept state — per-side. Both must be true before commit fires.
    auto setAcceptedBy(const CCharEntity* who) -> void;
    auto isAcceptedBy(const CCharEntity* who) const -> bool;
    auto bothAccepted() const -> bool;

    // Clear both accept flags. Retail semantics require any offer
    // change (takeSlot/releaseSlot) to invalidate prior acceptances;
    // the 0x034 packet handler calls this before applying the slot op.
    auto clearBothAcceptances() -> void;

    // Both accepted — commit and notify both clients. On success sends
    // ITEM_TRADE_RES::End to both; on failure sends Cancell. Cleans
    // tradePending and removeTransaction. Destroys *this — must be last op.
    // Returns true on successful commit.
    auto commitAndClose() -> bool;

    // Per-side slot operations. `who` must be initiator or partner.
    auto takeSlot(CCharEntity* who, std::uint8_t txSlot, std::uint8_t invSlot, std::uint32_t qty) -> bool;
    auto releaseSlot(const CCharEntity* who, std::uint8_t txSlot) -> void;

    // Read-only accessors for packet serialisers. Return defaults
    // when the tx is no longer Open.
    auto itemAt(const CCharEntity* who, std::uint8_t txSlot) const -> CItem*;
    auto offeredQtyAt(const CCharEntity* who, std::uint8_t txSlot) const -> std::uint32_t;

    struct Slot
    {
        CItem*        item{};
        std::uint8_t  invSlot{ 0xFF };
        std::uint32_t offeredQty{};
        ItemOwner     priorOwner{};
    };

    // Badge-gated public ctor so make_unique can reach it.
    PlayerTradeTransaction(xi::Badge<PlayerTradeTransaction>, CCharEntity* initiator, CCharEntity* partner);

    ~PlayerTradeTransaction() override
    {
        silentRollbackIfOpen();
    }

    auto holds(const CItem* item) const -> bool override;

protected:
    auto doCommit() -> bool override;
    auto doRollback() -> void override;

private:
    auto slotsFor(const CCharEntity* who) -> std::array<Slot, kMaxSlots>*;
    auto slotsFor(const CCharEntity* who) const -> const std::array<Slot, kMaxSlots>*;

    CCharEntity* initiator_{};
    CCharEntity* partner_{};
    bool         initiatorAccepted_{};
    bool         partnerAccepted_{};

    std::array<Slot, kMaxSlots> initiatorSlots_{};
    std::array<Slot, kMaxSlots> partnerSlots_{};
};
