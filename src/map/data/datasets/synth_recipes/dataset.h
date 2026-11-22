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
#include "data/enums/content.h"
#include "data/enums/element.h"
#include "data/enums/key_item.h"
#include "data/enums/skill_type.h"
#include "data/shared_types/recipe/dataset.h"

#include <array>
#include <string>
#include <string_view>
#include <vector>

namespace xi::data
{

inline constexpr std::array kCraftSkills{
    xi::SkillType::Woodworking,
    xi::SkillType::Smithing,
    xi::SkillType::Goldsmithing,
    xi::SkillType::Clothcraft,
    xi::SkillType::Leathercraft,
    xi::SkillType::Bonecraft,
    xi::SkillType::Alchemy,
    xi::SkillType::Cooking,
};

struct SynthRecipeData
{
    uint32                                 Id{};
    std::string                            Name{};
    bool                                   Desynth{};
    std::array<uint8, kCraftSkills.size()> Skills{}; // level needed in each of kCraftSkills, 0 when not needed
    xi::KeyItem                            KeyItem{};
    xi::Content                            Content{};
    xi::Element                            Crystal{};
    RecipeItemsData                        Items{};
};

using SynthRecipes = std::vector<SynthRecipeData>; // enabled recipes, ordered by id

} // namespace xi::data

namespace xi::data::datasets::synth_recipes::wire
{

struct Document;

}

namespace xi::data::datasets::synth_recipes
{

struct Dataset
{
    using Records      = SynthRecipes;
    using YamlDocument = wire::Document;

    static constexpr std::string_view kDataPath{ "synth_recipes" };
    static constexpr std::string_view kTitle{ "Synthesis Recipes" };
    static constexpr std::string_view kDescription{ "Synthesis and desynthesis recipes, grouped by the craft whose menu lists them." };

    static auto decode(std::string_view text) -> Records;
};

} // namespace xi::data::datasets::synth_recipes
