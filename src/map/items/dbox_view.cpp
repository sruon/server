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

#include "items/dbox_view.h"

#include "entities/charentity.h"
#include "items/item_store.h"

#include <algorithm>

CDboxView::CDboxView()
{
    Clean();
}

auto CDboxView::mode() const -> DboxMode
{
    return mode_;
}

void CDboxView::setMode(DboxMode m)
{
    mode_ = m;
}

void CDboxView::Clean()
{
    mode_ = DboxMode::None;
    items_.fill(nullptr);
}

void CDboxView::DrainItems(CCharEntity* owner)
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

bool CDboxView::IsSlotEmpty(const uint8 slotID) const
{
    return slotID >= items_.size() || items_[slotID] == nullptr;
}

bool CDboxView::SetItem(uint8 slotID, CItem* PItem)
{
    if (slotID >= items_.size())
    {
        return false;
    }
    items_[slotID] = PItem;
    return true;
}

void CDboxView::ClearSlot(uint8 slotID)
{
    if (slotID < items_.size())
    {
        items_[slotID] = nullptr;
    }
}

auto CDboxView::GetItem(uint8 slotID) const -> CItem*
{
    return slotID < items_.size() ? items_[slotID] : nullptr;
}

auto CDboxView::GetItemsCount() const -> uint8
{
    return static_cast<uint8>(std::ranges::count_if(items_, [](const CItem* p)
                                                    {
                                                        return p != nullptr;
                                                    }));
}
