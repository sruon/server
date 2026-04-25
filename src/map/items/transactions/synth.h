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

#include <array>
#include <cstdint>

struct CCraftState;

class CCharEntity;
class CItem;

// Custody for crystal + ingredients during a synth. Opens at startSynth
// once the recipe is validated. Crystal is consumed immediately (retail
// behavior); the tx stays Open through the animation window; commit on
// sendSynthDone releases remaining slots for synthutils to run its own
// decrements. Rollback restores everything on disconnect/zone.

class SynthTransaction : public Transaction
{
public:
    static constexpr std::size_t kMaxSlots = 9; // crystal + 8 ingredients

    static auto start(CCharEntity* player) -> SynthTransaction*;

    auto takeSlot(std::uint8_t txSlot, std::uint8_t invSlot) -> bool;

    // Claim crystal (slot 0) + ingredients (1..8) from the caller's
    // craftState. Empty ingredient slots are skipped.
    auto takeFromCraftState(const CCraftState& state) -> void;

    // Release the crystal from custody and UpdateItem(-1). Retail
    // consumes it the moment the synth starts.
    auto consumeCrystal() -> void;

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
