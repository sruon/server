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

#include "item_bindings.h"

#include "item.h"
#include "item_store.h"

auto CharacterInventoryBindings::equipBinding(uint8 slot) const -> const std::optional<xi::bindings::EquipBinding>&
{
    static const std::optional<xi::bindings::EquipBinding> kEmpty{};
    return slot < xi::bindings::kEquipBindingCount ? equip_[slot] : kEmpty;
}

auto CharacterInventoryBindings::bindings() const -> const std::vector<xi::bindings::Binding>&
{
    return bindings_;
}

auto CharacterInventoryBindings::isBoundItem(const CItem* item) const -> bool
{
    if (item == nullptr)
    {
        return false;
    }

    for (const auto& eb : equip_)
    {
        if (eb && eb->item == item)
        {
            return true;
        }
    }

    for (const auto& b : bindings_)
    {
        if (b.item == item)
        {
            return true;
        }
    }

    return false;
}

auto CharacterInventoryBindings::bindEquip(const uint8 equipSlot, const uint8 location, const uint8 invSlot, CItem* item) -> bool
{
    if (equipSlot >= xi::bindings::kEquipBindingCount || item == nullptr)
    {
        return false;
    }

    if (!ItemStore::isInInventory(item))
    {
        return false;
    }

    equip_[equipSlot] = xi::bindings::EquipBinding{ location, invSlot, item };
    return true;
}

auto CharacterInventoryBindings::clearEquip(uint8 equipSlot) -> void
{
    if (equipSlot >= xi::bindings::kEquipBindingCount)
    {
        return;
    }

    equip_[equipSlot].reset();
}

auto CharacterInventoryBindings::addBinding(xi::bindings::Binding binding) -> bool
{
    if (binding.item == nullptr || !ItemStore::isInInventory(binding.item))
    {
        return false;
    }

    for (const auto& existing : bindings_)
    {
        if (existing.reason == binding.reason && existing.location == binding.location && existing.invSlot == binding.invSlot)
        {
            return false;
        }
    }

    bindings_.push_back(binding);
    return true;
}

auto CharacterInventoryBindings::removeBinding(xi::bindings::Reason reason, uint8 location, uint8 invSlot) -> void
{
    for (auto it = bindings_.begin(); it != bindings_.end(); ++it)
    {
        if (it->reason == reason && it->location == location && it->invSlot == invSlot)
        {
            bindings_.erase(it);
            return;
        }
    }
}

auto CharacterInventoryBindings::addFurniture(uint8 location, uint8 invSlot, CItem* item, uint8 layoutSlot) -> bool
{
    return addBinding({ xi::bindings::Reason::Furniture, location, invSlot, item, layoutSlot });
}

auto CharacterInventoryBindings::removeFurniture(uint8 location, uint8 invSlot) -> void
{
    removeBinding(xi::bindings::Reason::Furniture, location, invSlot);
}

auto CharacterInventoryBindings::addBazaar(uint8 location, uint8 invSlot, CItem* item) -> bool
{
    return addBinding({ xi::bindings::Reason::Bazaar, location, invSlot, item, 0 });
}

auto CharacterInventoryBindings::removeBazaar(uint8 location, uint8 invSlot) -> void
{
    removeBinding(xi::bindings::Reason::Bazaar, location, invSlot);
}

auto CharacterInventoryBindings::updateItemLocation(uint8 oldLocation, uint8 oldSlot, uint8 newLocation, uint8 newSlot) -> void
{
    for (auto& eb : equip_)
    {
        if (eb && eb->location == oldLocation && eb->invSlot == oldSlot)
        {
            eb->location = newLocation;
            eb->invSlot  = newSlot;
        }
    }

    for (auto& b : bindings_)
    {
        if (b.location == oldLocation && b.invSlot == oldSlot)
        {
            b.location = newLocation;
            b.invSlot  = newSlot;
        }
    }
}
