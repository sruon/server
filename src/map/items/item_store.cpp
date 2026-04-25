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

#include "item_store.h"

#include "common/database.h"
#include "common/logging.h"

#include "item.h"
#include "item_container.h"
#include "item_equipment.h"
#include "item_weapon.h"
#include "transaction.h"

#include "entities/charentity.h"
#include "lua/luautils.h"
#include "packets/s2c/0x01e_item_num.h"
#include "packets/s2c/0x020_item_attr.h"
#include "utils/charutils.h"

#include <cstdlib>

auto ItemStore::destroy(const CItem* item) -> void
{
    if (item == nullptr)
    {
        return;
    }

    // Destroying a tx-held item would leave dangling tx accessors. Loud abort.
    if (isInTransaction(item))
    {
        ShowCriticalFmt("ItemStore::destroy: item {} is InTransaction; caller must release via the tx path first. Aborting.", item->getID());
        std::abort();
    }

    delete item; // cpp.sh allow
}

auto ItemStore::dropOwnedBy(CCharEntity* character, const CItem* item) -> void
{
    if (character != nullptr)
    {
        if (auto* tx = character->findTransactionHolding(item))
        {
            character->removeTransaction(tx);
        }
    }
    destroy(item);
}

auto ItemStore::writeOwner(CItem* item, ItemOwner newOwner) -> void
{
    if (item == nullptr)
    {
        return;
    }
    item->setOwner(std::move(newOwner), xi::Badge<ItemStore>{});
}

auto ItemStore::isInInventory(const CItem* item) -> bool
{
    return item != nullptr && std::holds_alternative<xi::item::InCharContainer>(item->owner());
}

auto ItemStore::isInTransaction(const CItem* item) -> bool
{
    return item != nullptr && std::holds_alternative<xi::item::InTransaction>(item->owner());
}

auto ItemStore::placeInInventory(CItem* item, uint32_t charId, uint8_t location, uint8_t slot) -> void
{
    if (item == nullptr)
    {
        return;
    }

    // Reject items held by a tx. Valid prior states are Unowned or InCharContainer.
    if (isInTransaction(item))
    {
        ShowWarning("ItemStore::placeInInventory: refusing to overwrite InTransaction owner for item %u", item->getID());
        return;
    }

    writeOwner(item, xi::item::InCharContainer{ charId, location, slot });
}

auto ItemStore::reslotInventory(CItem* item, uint8_t newLocation, uint8_t newSlot) -> void
{
    if (item == nullptr)
    {
        return;
    }

    const auto* current = std::get_if<xi::item::InCharContainer>(&item->owner());
    if (current == nullptr)
    {
        ShowWarning("ItemStore::reslotInventory: item %u not InCharContainer", item->getID());
        return;
    }

    writeOwner(item, xi::item::InCharContainer{ current->charId, newLocation, newSlot });
}

auto ItemStore::clearOwner(CItem* item) -> void
{
    if (item == nullptr)
    {
        return;
    }
    if (isInTransaction(item))
    {
        ShowWarning("ItemStore::clearOwner: refusing to clear InTransaction owner for item %u", item->getID());
        return;
    }
    writeOwner(item, xi::item::Unowned{});
}

auto ItemStore::moveToTransaction(CItem* item, Transaction* tx) -> std::optional<ItemOwner>
{
    if (item == nullptr || tx == nullptr || !tx->isOpen())
    {
        return std::nullopt;
    }
    ItemOwner prior = item->owner();
    if (!std::holds_alternative<xi::item::InCharContainer>(prior))
    {
        return std::nullopt;
    }

    writeOwner(item, xi::item::InTransaction{});
    return prior;
}

auto ItemStore::adoptIntoTransaction(CItem* item, Transaction* tx) -> std::optional<ItemOwner>
{
    if (item == nullptr || tx == nullptr || !tx->isOpen())
    {
        return std::nullopt;
    }

    // Only an Unowned item can enter a tx this way. Prevents silent
    // theft from another character or tx.
    if (!std::holds_alternative<xi::item::Unowned>(item->owner()))
    {
        return std::nullopt;
    }

    writeOwner(item, xi::item::InTransaction{});
    return ItemOwner{ xi::item::Unowned{} };
}

auto ItemStore::deliverToInventory(CItem* item, uint32_t charId, uint8_t location, uint8_t slot) -> bool
{
    if (item == nullptr || !isInTransaction(item))
    {
        ShowWarning("ItemStore::deliverToInventory: item %u not InTransaction", item ? item->getID() : 0);
        return false;
    }
    writeOwner(item, xi::item::InCharContainer{ charId, location, slot });
    return true;
}

auto ItemStore::restoreFromTransaction(CItem* item, ItemOwner priorOwner, xi::Badge<Transaction>) -> void
{
    writeOwner(item, std::move(priorOwner));
}

auto ItemStore::isBusy(const CCharEntity* owner, const CItem* item) -> bool
{
    if (item == nullptr)
    {
        return false;
    }

    if (isInTransaction(item))
    {
        return true;
    }

    return owner != nullptr && owner->bindings().isBoundItem(item);
}

auto ItemStore::consume(CCharEntity* owner, std::uint8_t location, std::uint8_t slot, std::uint32_t qty) -> std::uint16_t
{
    if (owner == nullptr || qty == 0)
    {
        return 0;
    }

    auto* container = owner->getStorage(location);
    if (container == nullptr)
    {
        return 0;
    }
    auto* item = container->GetItem(slot);
    if (item == nullptr)
    {
        return 0;
    }

    const std::uint16_t itemId     = item->getID();
    const std::uint32_t currentQty = item->getQuantity();
    if (qty > currentQty)
    {
        ShowDebug("ItemStore::consume: %s: qty %u > stack %u (item %u)", owner->getName(), qty, currentQty, itemId);
        return 0;
    }

    const std::uint32_t newQty = currentQty - qty;

    if (newQty > 0 || item->isType(ITEM_CURRENCY))
    {
        db::preparedStmt("UPDATE char_inventory SET quantity = ? WHERE charid = ? AND location = ? AND slot = ?",
                         newQty,
                         owner->id,
                         location,
                         slot);
        item->setQuantity(newQty);
        owner->pushPacket<GP_SERV_COMMAND_ITEM_NUM>(static_cast<CONTAINER_ID>(location), slot, newQty);
        return itemId;
    }

    // Stack hit zero — destroy the CItem. Run OnItemDrop and the
    // style-locked fallback before the local unique_ptr drops.
    db::preparedStmt("DELETE FROM char_inventory WHERE charid = ? AND location = ? AND slot = ?",
                     owner->id,
                     location,
                     slot);
    auto droppedItem = container->ReleaseItem(slot);
    owner->pushPacket<GP_SERV_COMMAND_ITEM_ATTR>(owner, nullptr, static_cast<CONTAINER_ID>(location), slot);

    // If this was the last copy of a style-locked item, revert the
    // appearance so the client stops rendering the ghost style.
    if (owner->getStyleLocked() && !charutils::HasItem(owner, itemId))
    {
        if (item->isType(ITEM_WEAPON))
        {
            if (owner->styleItems[SLOT_MAIN] == itemId)
            {
                charutils::UpdateWeaponStyle(owner, SLOT_MAIN, static_cast<CItemWeapon*>(owner->getEquip(SLOT_MAIN)));
            }
            else if (owner->styleItems[SLOT_SUB] == itemId)
            {
                charutils::UpdateWeaponStyle(owner, SLOT_SUB, static_cast<CItemWeapon*>(owner->getEquip(SLOT_SUB)));
            }
        }
        else if (item->isType(ITEM_EQUIPMENT))
        {
            const auto equipSlotID = static_cast<CItemEquipment*>(item)->getSlotType();
            if (owner->styleItems[equipSlotID] == itemId)
            {
                switch (equipSlotID)
                {
                    case SLOT_HEAD:
                    case SLOT_BODY:
                    case SLOT_HANDS:
                    case SLOT_LEGS:
                    case SLOT_FEET:
                        charutils::UpdateArmorStyle(owner, equipSlotID);
                        break;
                    default:
                        break;
                }
            }
        }
    }

    luautils::OnItemDrop(owner, item);
    return itemId;
}
