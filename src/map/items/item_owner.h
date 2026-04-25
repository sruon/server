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

#include <cstdint>
#include <variant>

// Where an item currently lives. Exactly one variant at any moment.

namespace xi::item
{
// In one of the character's containers (inventory, mog safe, wardrobe, etc).
struct InCharContainer
{
    uint32_t charId{};
    uint8_t  location{};
    uint8_t  slot{};
};

// Held by a live Transaction. Look up via character->activeTransaction<T>().
struct InTransaction
{
};

// Freshly constructed, not placed anywhere. ItemStore::place* promotes it.
struct Unowned
{
};
} // namespace xi::item

using ItemOwner = std::variant<
    xi::item::Unowned,
    xi::item::InCharContainer,
    xi::item::InTransaction>;
