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

#include "common/cbasetypes.h"

#include <array>

// CDboxView — 16-slot view buffer for an open delivery-box window.
// Mode distinguishes Send (outbound mail) from Recv (incoming).

enum class DboxMode : uint8
{
    None = 0,
    Send,
    Recv,
};

constexpr std::size_t DBOX_SLOT_COUNT = 16;

class CItem;
class CCharEntity;

class CDboxView
{
public:
    CDboxView();

    auto mode() const -> DboxMode;
    void setMode(DboxMode m);

    // Slot-level ops over the 16-slot view.
    bool SetItem(uint8 slotID, CItem* PItem);
    void ClearSlot(uint8 slotID);
    bool IsSlotEmpty(uint8 slotID) const;
    auto GetItem(uint8 slotID) const -> CItem*;
    auto GetItemsCount() const -> uint8;

    // Reset the view (mode=None, all slots cleared). Does NOT destroy
    // items; callers must DrainItems() first when items need freeing.
    void Clean();

    // Drop every held CItem via ItemStore::dropOwnedBy so any live
    // DboxViewTransaction custody is torn down before the CItem is freed.
    void DrainItems(CCharEntity* owner);

private:
    DboxMode                            mode_{ DboxMode::None };
    std::array<CItem*, DBOX_SLOT_COUNT> items_{};
};
