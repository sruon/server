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

#include "dbox_view_transaction.h"

#include "entities/charentity.h"
#include "items/item.h"
#include "items/item_store.h"

auto DboxViewTransaction::start(CCharEntity* owner, CItem* item) -> TransactionHandle<DboxViewTransaction>
{
    if (owner == nullptr || item == nullptr)
    {
        return {};
    }

    // Reject if some tx on the character already holds this item
    // (zombie-stamping guard — SendOldItems double-fire could try to
    // re-register an item that's still tracked from a prior load).
    if (owner->findTransactionHolding(item) != nullptr)
    {
        return {};
    }

    auto tx = std::make_unique<DboxViewTransaction>(xi::Badge<DboxViewTransaction>{});

    // Direct Unowned → InTransaction — skip the ghost-InCharContainer
    // stamp. A save-path race between placeInInventory and moveToTransaction
    // could otherwise persist this dbox item to char_inventory at
    // slot 0.
    if (!ItemStore::adoptIntoTransaction(item, tx.get()))
    {
        tx->rollback();
        return {};
    }
    tx->item_ = item;

    return owner->addTransaction(std::move(tx));
}

auto DboxViewTransaction::holds(const CItem* item) const -> bool
{
    return isOpen() && item != nullptr && item_ == item;
}

auto DboxViewTransaction::doCommit() -> bool
{
    return true;
}
