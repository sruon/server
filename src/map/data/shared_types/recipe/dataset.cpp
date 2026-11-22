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

#include "data/shared_types/recipe/yaml.h"

#include <fmt/format.h>

#include <stdexcept>

namespace xi::data::shared
{

auto toRecipeItems(const RecipeIngredients& ingredients, const RecipeResults& results, const std::string_view context) -> RecipeItemsData
{
    RecipeItemsData items;

    size_t total = 0;
    for (const auto& [item, count] : ingredients)
    {
        if (count == 0)
        {
            throw std::runtime_error(fmt::format("{} lists {} with a count of 0", context, item));
        }

        total += count;
        items.Ingredients.emplace_back(item, count);
    }

    if (total == 0 || total > 8)
    {
        throw std::runtime_error(fmt::format("{} needs 1 to 8 ingredients, has {}", context, total));
    }

    if (!results.nq.item)
    {
        throw std::runtime_error(fmt::format("{} has no normal quality item", context));
    }

    const std::array tiers{ std::optional{ results.nq }, results.hq1, results.hq2, results.hq3 };

    RecipeResultData previous{ .Quantity = 1 };
    for (size_t tier = 0; tier < tiers.size(); ++tier)
    {
        if (tiers[tier])
        {
            previous.Item     = tiers[tier]->item.value_or(previous.Item);
            previous.Quantity = tiers[tier]->qty.value_or(previous.Quantity);
        }

        if (previous.Quantity == 0)
        {
            throw std::runtime_error(fmt::format("{} makes 0 items", context));
        }

        items.Results[tier] = previous;
    }

    return items;
}

} // namespace xi::data::shared
