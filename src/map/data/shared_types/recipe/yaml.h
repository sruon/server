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
#include "data/shared_types/recipe/dataset.h"

#include <glaze/glaze.hpp>

#include <map>
#include <optional>
#include <string>
#include <string_view>

namespace xi::data::shared
{

struct RecipeResult
{
    std::optional<std::string> item;
    std::optional<uint8>       qty;
};

struct RecipeResults
{
    RecipeResult                nq;
    std::optional<RecipeResult> hq1;
    std::optional<RecipeResult> hq2;
    std::optional<RecipeResult> hq3;
};

using RecipeIngredients = std::map<std::string, uint8>;

auto toRecipeItems(const RecipeIngredients& ingredients, const RecipeResults& results, std::string_view context) -> RecipeItemsData;

} // namespace xi::data::shared

template <>
struct glz::json_schema<xi::data::shared::RecipeResult>
{
    glz::schema item{ .description = "Item name, or its id when several items share the name. Defaults to the previous tier's item." };
    glz::schema qty{ .description = "How many are made. Defaults to the previous tier's quantity, 1 for normal quality.", .minimum = 1L, .maximum = 99L };
};

template <>
struct glz::json_schema<xi::data::shared::RecipeResults>
{
    glz::schema nq{ .description = "Normal quality result. Needs an item." };
    glz::schema hq1{ .description = "High quality tier 1. Omit when it matches normal quality." };
    glz::schema hq2{ .description = "High quality tier 2. Omit when it matches tier 1." };
    glz::schema hq3{ .description = "High quality tier 3. Omit when it matches tier 2." };
};
