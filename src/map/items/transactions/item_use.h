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

// Custody for a single-item use (potion, scroll, charged equipment).
//
// Consumable items move to InTransaction at open; commit decrements qty
// (destroys on last). Charged equipment (takesCustody == No) stays in
// inventory — the charge decrement is in applyEquipmentItemUse, commit
// and rollback are no-ops.

class ItemUseTransaction : public Transaction
{
public:
    // Owned by CItemState::tx_ so lifetime matches the item-use action.
    static auto start(CCharEntity* player, CItemUsable* item, uint8_t location, uint8_t slot) -> std::unique_ptr<ItemUseTransaction>;

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
