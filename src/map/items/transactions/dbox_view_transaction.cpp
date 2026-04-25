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

auto DboxViewTransaction::start(CCharEntity* owner, CItem* item) -> DboxViewTransaction*
{
    if (owner == nullptr || item == nullptr)
    {
        return {};
    }

    // Guard against SendOldItems double-fire re-registering an item
    // already tracked from a prior load.
    if (owner->findTransactionHolding(item) != nullptr)
    {
        return {};
    }

    auto tx = std::unique_ptr<DboxViewTransaction>(new DboxViewTransaction());

    // adoptIntoTransaction (Unowned → InTransaction) — skips the
    // InCharContainer step to avoid a save-path race persisting a
    // dbox item into char_inventory at slot 0.
    auto prior = ItemStore::adoptIntoTransaction(item, tx.get());
    if (!prior)
    {
        tx->rollback();
        return {};
    }

    tx->item_       = item;
    tx->priorOwner_ = std::move(*prior);

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

auto DboxViewTransaction::doRollback() -> void
{
    if (item_ != nullptr)
    {
        restoreEntry(item_, priorOwner_);
        item_ = nullptr;
    }
}
