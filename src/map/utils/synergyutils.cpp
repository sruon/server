/*
===========================================================================

  Copyright (c) 2024 LandSandBoat Dev Teams

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
#include "synergyutils.h"

#include "common/logging.h"
#include "common/utils.h"
#include "data/datasets/synergy_recipes/dataset.h"
#include "data/enums/skill_type.h"
#include "utils/dataset_loader.h"
#include "utils/itemutils.h"

#include <algorithm>
#include <array>
#include <stdexcept>
#include <utility>
#include <vector>

namespace synergyutils
{

namespace
{

std::vector<SynergyRecipe> synergyRecipes;

// Skill numbering the synergy scripts use: 0 synergy, 1 fishing, then woodworking to cooking.
auto synergySkill(const xi::SkillType skill) -> uint8
{
    if (skill == xi::SkillType::Synergy)
    {
        return 0;
    }

    return static_cast<uint8>(skill) - static_cast<uint8>(xi::SkillType::Fishing) + 1;
}

} // namespace

void LoadSynergyRecipes()
{
    std::vector<SynergyRecipe> recipes;

    try
    {
        for (const auto& source : xi::data::loadDataset<xi::data::datasets::synergy_recipes::Dataset>())
        {
            const auto context = fmt::format("synergy recipe {}", source.Id);

            std::array<uint16, 8> ingredients{};
            size_t                filled = 0;
            for (const auto& [item, count] : source.Items.Ingredients)
            {
                std::fill_n(ingredients.begin() + filled, count, xi::items::resolveRecipeItem(item, context));
                filled += count;
            }

            std::sort(ingredients.begin(), ingredients.begin() + filled);

            std::array<std::pair<uint8, uint8>, 2> support{}; // skill and rank
            for (size_t index = 0; index < source.Support.size(); ++index)
            {
                support[index] = { synergySkill(source.Support[index].Skill), static_cast<uint8>(source.Support[index].Rank) };
            }

            const auto& results = source.Items.Results;
            const auto& fewell  = source.Fewell;

            recipes.push_back({
                .id = source.Id,

                .primary_skill   = synergySkill(xi::SkillType::Synergy),
                .primary_rank    = static_cast<uint8>(source.Rank),
                .secondary_skill = support[0].first,
                .secondary_rank  = support[0].second,
                .tertiary_skill  = support[1].first,
                .tertiary_rank   = support[1].second,

                .cost_fire_fewell      = fewell[0],
                .cost_ice_fewell       = fewell[1],
                .cost_wind_fewell      = fewell[2],
                .cost_earth_fewell     = fewell[3],
                .cost_lightning_fewell = fewell[4],
                .cost_water_fewell     = fewell[5],
                .cost_light_fewell     = fewell[6],
                .cost_dark_fewell      = fewell[7],
                .ingredients           = ingredients,

                .result    = xi::items::resolveRecipeItem(results[0].Item, context),
                .resultHQ1 = xi::items::resolveRecipeItem(results[1].Item, context),
                .resultHQ2 = xi::items::resolveRecipeItem(results[2].Item, context),
                .resultHQ3 = xi::items::resolveRecipeItem(results[3].Item, context),

                .resultQty    = results[0].Quantity,
                .resultHQ1Qty = results[1].Quantity,
                .resultHQ2Qty = results[2].Quantity,
                .resultHQ3Qty = results[3].Quantity,

                .resultName = source.Name,
            });
        }
    }
    catch (const std::exception& error)
    {
        ShowErrorFmt("Failed to load synergy recipes: {}", error.what());
        return;
    }

    synergyRecipes = std::move(recipes);
}

auto GetSynergyRecipeByID(uint32 id) -> Maybe<SynergyRecipe>
{
    // TODO: Use a map instead of a vector for faster lookups
    if (const auto it = std::ranges::find(synergyRecipes, id, &SynergyRecipe::id); it != synergyRecipes.end())
    {
        return *it;
    }

    return std::nullopt;
}

auto GetSynergyRecipeByIngredients(const std::array<uint16, 8>& ingredients) -> Maybe<SynergyRecipe>
{
    if (const auto it = std::ranges::find(synergyRecipes, ingredients, &SynergyRecipe::ingredients); it != synergyRecipes.end())
    {
        return *it;
    }

    return std::nullopt;
}

} // namespace synergyutils
