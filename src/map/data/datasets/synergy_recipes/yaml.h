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
#include "data/enums/element.h"
#include "data/enums/skill_type.h"
#include "data/shared_types/recipe/yaml.h"
#include "data/yaml/enum_keyed_map.h"
#include "data/yaml/enum_token.h"
#include "data/yaml/schema_annotations.h"

#include <glaze/glaze.hpp>

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace xi::data::datasets::synergy_recipes::wire
{

struct Support
{
    yaml::EnumToken<xi::SkillType> skill;
    yaml::EnumToken<xi::CraftRank> rank;
};

struct Recipe
{
    std::string                                           name;
    std::optional<yaml::EnumToken<xi::CraftRank>>         rank;
    std::optional<std::vector<Support>>                   support;
    std::optional<yaml::EnumKeyedMap<xi::Element, uint8>> fewell;
    shared::RecipeIngredients                             ingredients;
    shared::RecipeResults                                 results;
};

struct Document
{
    std::map<uint32, Recipe> synergy_recipes;

    using YamlRoot = yaml::DatasetRoot<&Document::synergy_recipes>;
};

} // namespace xi::data::datasets::synergy_recipes::wire

template <>
struct glz::json_schema<xi::data::datasets::synergy_recipes::wire::Support>
{
    glz::schema skill{ .description = "Craft whose rank is needed." };
    glz::schema rank{ .description = "Rank needed in that craft." };
};

template <>
struct glz::json_schema<xi::data::datasets::synergy_recipes::wire::Recipe>
{
    glz::schema name{ .description = "Name of the result." };
    glz::schema rank{ .description = "Synergy rank needed, amateur when omitted." };
    glz::schema support{ .description = "Up to two other crafts and the rank needed in each.", .maxItems = 2UL };
    glz::schema fewell{ .description = "Fewell of each element the furnace burns." };
    glz::schema ingredients{ .description = "Ingredients and how many of each, 8 at most in all. Item names, or the id when several items share the name." };
    glz::schema results{ .description = "What each quality tier makes." };
};

template <>
struct glz::json_schema<xi::data::datasets::synergy_recipes::wire::Document>
{
    glz::schema synergy_recipes{ .description = "Synergy recipes keyed by recipe id. The furnace stores the id while it works." };
};
