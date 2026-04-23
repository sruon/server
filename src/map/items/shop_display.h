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

// CShopDisplay — listing buffer for an NPC shop window.
// Populated by Lua (`createShop` + `addShopItem`), serialized by the
// 0x03c / 0x03e / 0x083 / 0x084 / 0x085 packet handlers. The
// guild-gate fields are serialized into the shop-list packet for the
// client to display; server-side guild-rank enforcement happens at
// the buy handler only if added there.

struct CShopEntry
{
    uint16 itemId{};
    uint32 price{};
    uint8  guildId{};
    uint16 guildRank{};
};

class CShopDisplay
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

    // Append. No random-insertion — entries are filled in order.
    void push(uint16 itemId, uint32 price, uint8 guildId = 0, uint16 guildRank = 0)
    {
        entries_.push_back(CShopEntry{ itemId, price, guildId, guildRank });
    }

    // Clear everything (used on shop close).
    void clear()
    {
        entries_.clear();
        clearPendingSell();
    }

    // ---- Pending sell staging -----------------------------------------
    //
    // When a player offers an item TO a shop, 0x084 stores what was
    // offered here; 0x085 reads it back and finalizes the sale. Only
    // one offer at a time. This is a separate lane from the shop
    // listing entries — it doesn't participate in itemsCount() or the
    // 0x03c packet.

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
