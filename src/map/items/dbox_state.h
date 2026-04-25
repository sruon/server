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

// 16-slot view of an open delivery-box window.

enum class DboxMode : uint8
{
    None = 0,
    Send,
    Recv,
};

constexpr std::size_t DBOX_SLOT_COUNT = 16;

class CItem;
class CCharEntity;

class CDboxState
{
public:
    CDboxState();

    auto mode() const -> DboxMode;
    void setMode(DboxMode m);

    bool SetItem(uint8 slotID, CItem* PItem);
    void ClearSlot(uint8 slotID);
    bool IsSlotEmpty(uint8 slotID) const;
    auto GetItem(uint8 slotID) const -> CItem*;
    auto GetItemsCount() const -> uint8;

    // Reset mode + clear slots. Does not free items; call DrainItems first.
    void Clean();

    // Drop every held CItem via ItemStore::dropOwnedBy so any live
    // DboxViewTransaction is retired before the CItem is freed.
    void DrainItems(CCharEntity* owner);

private:
    DboxMode                            mode_{ DboxMode::None };
    std::array<CItem*, DBOX_SLOT_COUNT> items_{};
};
