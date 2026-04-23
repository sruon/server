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
#include "common/types/flag.h"
#include "items/item_owner.h"
#include "items/transaction.h"

#include <cstdint>
#include <memory>

class CCharEntity;
class CItemUsable;

using TakesCustody = xi::Flag<struct TakesCustodyTag>;

// ItemUseTransaction — custody + atomic commit for a single-item use (potion,
// scroll, Warp Cudgel charge, Soultrapper, etc.).
//
// Two shapes:
//   - Consumable  : item moves InCharContainer → InTransaction at open.
//                   On commit, item returns to inventory and its
//                   quantity is decremented by 1 (destroyed if last).
//                   On rollback, item returns to inventory, qty
//                   unchanged.
//   - Charged equipment (takesCustody == No):
//                   item stays InCharContainer the whole time (equipment
//                   bindings still point to it). Commit is a no-op —
//                   the charge decrement is the existing
//                   applyEquipmentItemUse path's responsibility.
//                   Rollback is a no-op.

class ItemUseTransaction : public Transaction
{
public:
    // Returns std::unique_ptr (not TransactionHandle) because this tx
    // is owned by CItemState::tx_ for the duration of the item-use
    // action, not by CCharEntity::transactions. CItemState relies on
    // unique_ptr semantics to auto-rollback when its own lifetime ends
    // (interrupt, cancel, zone change).
    static auto start(CCharEntity* player, CItemUsable* item, uint8_t location, uint8_t slot) -> std::unique_ptr<ItemUseTransaction>;

    // Public-but-badge-gated so std::make_unique can reach the ctor.
    // Only ItemUseTransaction itself can default-construct xi::Badge<ItemUseTransaction>.
    ItemUseTransaction(xi::Badge<ItemUseTransaction>, CCharEntity* player, CItemUsable* item, uint8_t location, uint8_t slot, TakesCustody takesCustody);

    ~ItemUseTransaction() override
    {
        silentRollbackIfOpen();
    }

    auto holds(const CItem* item) const -> bool override;

protected:
    auto doCommit() -> bool override;
    auto doRollback() -> void override;

private:
    CCharEntity* player_{};
    CItemUsable* item_{};
    uint8_t      location_{};
    uint8_t      slot_{};
    TakesCustody takesCustody_{ false };
    ItemOwner    priorOwner_{};
};
