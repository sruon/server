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

#include "item_use.h"

#include "entities/charentity.h"
#include "items/item.h"
#include "items/item_store.h"
#include "items/item_usable.h"

ItemUseTransaction::ItemUseTransaction(xi::Badge<ItemUseTransaction>, CCharEntity* player, CItemUsable* item, uint8_t location, uint8_t slot, TakesCustody takesCustody)
: Transaction()
, player_(player)
, item_(item)
, location_(location)
, slot_(slot)
, takesCustody_(takesCustody)
{
}

auto ItemUseTransaction::start(CCharEntity* player, CItemUsable* item, uint8_t location, uint8_t slot) -> std::unique_ptr<ItemUseTransaction>
{
    if (player == nullptr || item == nullptr)
    {
        return nullptr;
    }

    // Equipment with charges stays equipped during use; the tx carries
    // the state machine but doesn't take custody.
    const bool         isEquipment  = item->isType(ITEM_EQUIPMENT) || item->isType(ITEM_WEAPON);
    const TakesCustody takesCustody = isEquipment ? TakesCustody::No : TakesCustody::Yes;

    auto tx = std::make_unique<ItemUseTransaction>(xi::Badge<ItemUseTransaction>{}, player, item, location, slot, takesCustody);

    if (takesCustody)
    {
        auto prior = ItemStore::moveToTransaction(item, tx.get());
        if (!prior)
        {
            tx->rollback();
            return nullptr;
        }
        tx->priorOwner_ = std::move(*prior);
    }
    return tx;
}

auto ItemUseTransaction::holds(const CItem* item) const -> bool
{
    return isOpen() && takesCustody_ && item != nullptr && item_ == item;
}

auto ItemUseTransaction::doRollback() -> void
{
    if (takesCustody_ && item_ != nullptr)
    {
        restoreEntry(item_, priorOwner_);
        item_ = nullptr;
    }
}

auto ItemUseTransaction::doCommit() -> bool
{
    if (!takesCustody_)
    {
        // Charged equipment: nothing to do. applyEquipmentItemUse
        // handles the charge decrement through the existing path.
        return true;
    }

    if (item_ == nullptr)
    {
        return false;
    }

    // Hand the stack back to its inventory slot, then decrement by 1.
    // ItemStore::consume owns the drop-on-zero, DB write, and packet
    // push — no charutils::UpdateItem dependency from inside the tx.
    ItemStore::deliverToInventory(item_, player_->id, location_, slot_);
    ItemStore::consume(player_, location_, slot_, 1);
    item_ = nullptr;
    return true;
}
