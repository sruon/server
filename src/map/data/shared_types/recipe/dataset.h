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
#include <string>
#include <utility>
#include <vector>

namespace xi::data
{

// Items stay names here; the loader resolves them once the item table exists. A name shared by several items is written as its id.

struct RecipeResultData
{
    std::string Item{};
    uint8       Quantity{};
};

struct RecipeItemsData
{
    std::vector<std::pair<std::string, uint8>> Ingredients{}; // item and count
    std::array<RecipeResultData, 4>            Results{};     // normal quality, then high quality tiers 1 to 3
};

} // namespace xi::data
