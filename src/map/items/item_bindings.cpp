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

auto CItemBindings::equipBinding(uint8 slot) const -> const std::optional<xi::bindings::EquipBinding>&
{
    static const std::optional<xi::bindings::EquipBinding> kEmpty{};
    return slot < xi::bindings::kEquipBindingCount ? equip_[slot] : kEmpty;
}

auto CItemBindings::bindEquip(const uint8 equipSlot, const uint8 location, const uint8 invSlot, CItem* item) -> bool
{
    if (equipSlot >= xi::bindings::kEquipBindingCount || item == nullptr)
    {
        return false;
    }
    // Refuse if item is in a tx (custody is elsewhere) — B1 violation.
    if (!ItemStore::isInInventory(item))
    {
        return false;
    }
    equip_[equipSlot] = xi::bindings::EquipBinding{ location, invSlot, item };
    item->setBinding(true, xi::Badge<CItemBindings>{});
    return true;
}

auto CItemBindings::clearEquip(uint8 equipSlot) -> void
{
    if (equipSlot >= xi::bindings::kEquipBindingCount)
    {
        return;
    }
    if (equip_[equipSlot])
    {
        if (auto* item = equip_[equipSlot]->item)
        {
            item->setBinding(false, xi::Badge<CItemBindings>{});
        }
        equip_[equipSlot].reset();
    }
}

auto CItemBindings::addFurniture(xi::bindings::FurnitureBinding binding) -> bool
{
    if (binding.item == nullptr || !ItemStore::isInInventory(binding.item))
    {
        return false;
    }
    binding.item->setBinding(true, xi::Badge<CItemBindings>{});
    furniture_.push_back(binding);
    return true;
}

auto CItemBindings::removeFurniture(uint8 location, uint8 invSlot) -> void
{
    for (auto it = furniture_.begin(); it != furniture_.end(); ++it)
    {
        if (it->location == location && it->invSlot == invSlot)
        {
            if (it->item != nullptr)
            {
                it->item->setBinding(false, xi::Badge<CItemBindings>{});
            }
            furniture_.erase(it);
            return;
        }
    }
}

auto CItemBindings::updateItemLocation(uint8 oldLocation, uint8 oldSlot, uint8 newLocation, uint8 newSlot) -> void
{
    for (auto& eb : equip_)
    {
        if (eb && eb->location == oldLocation && eb->invSlot == oldSlot)
        {
            eb->location = newLocation;
            eb->invSlot  = newSlot;
        }
    }
    for (auto& fb : furniture_)
    {
        if (fb.location == oldLocation && fb.invSlot == oldSlot)
        {
            fb.location = newLocation;
            fb.invSlot  = newSlot;
        }
    }
}
