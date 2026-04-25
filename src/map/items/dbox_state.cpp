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

#include "items/dbox_state.h"

#include "entities/charentity.h"
#include "items/item_store.h"

#include <algorithm>

CDboxState::CDboxState()
{
    Clean();
}

auto CDboxState::mode() const -> DboxMode
{
    return mode_;
}

void CDboxState::setMode(DboxMode m)
{
    mode_ = m;
}

void CDboxState::Clean()
{
    mode_ = DboxMode::None;
    items_.fill(nullptr);
}

void CDboxState::DrainItems(CCharEntity* owner)
{
    if (owner == nullptr)
    {
        return;
    }
    for (auto& slot : items_)
    {
        if (slot != nullptr)
        {
            ItemStore::dropOwnedBy(owner, slot);
            slot = nullptr;
        }
    }
}

bool CDboxState::IsSlotEmpty(const uint8 slotID) const
{
    return slotID >= items_.size() || items_[slotID] == nullptr;
}

bool CDboxState::SetItem(uint8 slotID, CItem* PItem)
{
    if (slotID >= items_.size())
    {
        return false;
    }
    items_[slotID] = PItem;
    return true;
}

void CDboxState::ClearSlot(uint8 slotID)
{
    if (slotID < items_.size())
    {
        items_[slotID] = nullptr;
    }
}

auto CDboxState::GetItem(uint8 slotID) const -> CItem*
{
    return slotID < items_.size() ? items_[slotID] : nullptr;
}

auto CDboxState::GetItemsCount() const -> uint8
{
    return static_cast<uint8>(std::ranges::count_if(items_, [](const CItem* p)
                                                    {
                                                        return p != nullptr;
                                                    }));
}
