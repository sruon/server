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

#include <vector>

// Listing buffer for an NPC shop window. Populated by createShop +
// addShopItem in Lua; read by the 0x03c/0x03e/0x083-085 handlers.

struct CShopEntry
{
    uint16 itemId{};
    uint32 price{};
    uint8  guildId{};
    uint16 guildRank{};
};

class CShopState
{
public:
    auto itemsCount() const -> uint8
    {
        return static_cast<uint8>(entries_.size());
    }

    auto entry(uint8 slot) const -> const CShopEntry&
    {
        return entries_.at(slot);
    }

    void reset(uint8 initialSize)
    {
        entries_.clear();
        entries_.reserve(initialSize);
    }

    void push(uint16 itemId, uint32 price, uint8 guildId = 0, uint16 guildRank = 0)
    {
        entries_.push_back(CShopEntry{ itemId, price, guildId, guildRank });
    }

    void clear()
    {
        entries_.clear();
        clearPendingSell();
    }

    // Sell offer staged by 0x084 for 0x085 to finalize. Separate from
    // listing entries — one pending offer at a time.

    struct PendingSell
    {
        uint16 itemId{};
        uint8  invSlot{ 0xFF };
        uint32 quantity{};
    };

    auto pendingSell() const -> const PendingSell&
    {
        return pendingSell_;
    }

    void stagePendingSell(uint16 itemId, uint8 invSlot, uint32 quantity)
    {
        pendingSell_ = PendingSell{ itemId, invSlot, quantity };
    }

    void clearPendingSell()
    {
        pendingSell_ = PendingSell{};
    }

private:
    std::vector<CShopEntry> entries_;
    PendingSell             pendingSell_{};
};
