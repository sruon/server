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
#include "data/enums/craft_rank.h"
#include "data/enums/skill_type.h"
#include "data/shared_types/recipe/dataset.h"

#include <array>
#include <string>
#include <string_view>
#include <vector>

namespace xi::data
{

struct SynergySupportData
{
    xi::SkillType Skill{};
    xi::CraftRank Rank{};
};

struct SynergyRecipeData
{
    uint32                          Id{};
    std::string                     Name{};
    xi::CraftRank                   Rank{}; // synergy rank
    std::vector<SynergySupportData> Support{};
    std::array<uint8, 8>            Fewell{}; // fewell of each element, fire to dark
    RecipeItemsData                 Items{};
};

using SynergyRecipes = std::vector<SynergyRecipeData>; // ordered by id

} // namespace xi::data

namespace xi::data::datasets::synergy_recipes::wire
{

struct Document;

}

namespace xi::data::datasets::synergy_recipes
{

struct Dataset
{
    using Records      = SynergyRecipes;
    using YamlDocument = wire::Document;

    static constexpr std::string_view kDataPath{ "synergy_recipes" };
    static constexpr std::string_view kTitle{ "Synergy Recipes" };
    static constexpr std::string_view kDescription{ "Synergy furnace recipes and the fewell they burn." };

    static auto decode(std::string_view text) -> Records;
};

} // namespace xi::data::datasets::synergy_recipes
