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

struct CCraftState;

class CCharEntity;
class CItem;

// SynthTransaction — custody for crystal + ingredients during a synth.
//
// Opened at startSynth once the recipe has been validated. Each
// ingredient slot (and the crystal) transitions InCharContainer →
// InTransaction via ItemStore::moveToTransaction, preventing the player from
// selling/trading/dropping them mid-animation.
//
// Lifecycle:
//   startSynth → open() → takeSlot for crystal + each ingredient
//                → deliverToInventory(crystal) + UpdateItem(-1) to
//                  consume the crystal immediately (retail behaviour:
//                  crystal is gone before the animation plays)
//                → tx stays Open through the animation window
//   sendSynthDone (success or fail) → commit()
//                → deliverToInventory for every remaining slot
//                → synthutils runs its own UpdateItem decrements
//   zone / disconnect / crash → dtor-rollback → items restored
//
// Partial-custody model: the tx takes the whole CItem stack for each
// slot (matches legacy "reserve 1 per slot" semantics).  Unlike
// NpcTradeTransaction the tx doesn't drive the decrement itself — synthutils
// owns the break/success math.  SynthTransaction only guarantees that items
// re-enter InCharContainer via deliverToInventory before synthutils calls
// UpdateItem.

class SynthTransaction : public Transaction
{
public:
    static constexpr std::size_t kMaxSlots = 9; // crystal + 8 ingredients

    static auto start(CCharEntity* player) -> TransactionHandle<SynthTransaction>;

    // Move the item at inventory `invSlot` into the tx at `txSlot`.
    auto takeSlot(std::uint8_t txSlot, std::uint8_t invSlot) -> bool;

    // Claim the crystal (tx slot 0) and every ingredient (tx slots
    // 1-8) from the caller's craftState. Empty ingredient slots
    // (invSlot == 0xFF) are skipped.
    auto takeFromCraftState(const CCraftState& state) -> void;

    // Retail: the crystal is consumed the moment the synth starts,
    // before the animation plays. consumeCrystal handles both halves
    // atomically — release the crystal from tx custody, then
    // UpdateItem(-1) on its inventory slot. Crystal lives at slot 0.
    auto consumeCrystal() -> void;

    // Badge-gated public ctor so make_unique can reach it.
    SynthTransaction(xi::Badge<SynthTransaction>, CCharEntity* player);

    ~SynthTransaction() override
    {
        silentRollbackIfOpen();
    }

    auto holds(const CItem* item) const -> bool override;

protected:
    auto doCommit() -> bool override;
    auto doRollback() -> void override;

private:
    auto releaseSlot(std::uint8_t txSlot) -> void;

    struct Slot
    {
        CItem*       item{};
        std::uint8_t invSlot{ 0xFF };
        ItemOwner    priorOwner{};
    };

    CCharEntity*                player_{};
    std::array<Slot, kMaxSlots> slots_{};
};
