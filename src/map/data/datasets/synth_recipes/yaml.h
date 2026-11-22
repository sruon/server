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
#include "data/shared_types/recipe/yaml.h"
#include "data/yaml/enum_keyed_map.h"
#include "data/yaml/enum_token.h"
#include "data/yaml/schema_annotations.h"

#include <glaze/glaze.hpp>

#include <map>
#include <optional>
#include <string>

namespace xi::data::datasets::synth_recipes::wire
{

struct Recipe
{
    std::string                                             name;
    std::optional<bool>                                     enabled;
    std::optional<bool>                                     desynth;
    uint8                                                   level{};
    std::optional<yaml::EnumKeyedMap<xi::SkillType, uint8>> subcrafts;
    std::optional<yaml::EnumToken<xi::KeyItem>>             key_item;
    std::optional<yaml::EnumToken<xi::Content>>             content;
    yaml::EnumToken<xi::Element>                            crystal;
    shared::RecipeIngredients                               ingredients;
    shared::RecipeResults                                   results;
};

using Recipes = std::optional<std::map<uint32, Recipe>>;

struct Crafts
{
    Recipes woodworking;
    Recipes smithing;
    Recipes goldsmithing;
    Recipes clothcraft;
    Recipes leathercraft;
    Recipes bonecraft;
    Recipes alchemy;
    Recipes cooking;
};

struct Document
{
    Crafts synth_recipes;

    using YamlRoot = yaml::DatasetRoot<&Document::synth_recipes>;
};

} // namespace xi::data::datasets::synth_recipes::wire

template <>
struct glz::json_schema<xi::data::datasets::synth_recipes::wire::Recipe>
{
    glz::schema name{ .description = "Name written to the craft audit log, usually the result." };
    glz::schema enabled{ .description = "Set to false to keep a recipe nobody can craft.", .defaultValue = true };
    glz::schema desynth{ .description = "A desynthesis recipe.", .defaultValue = false };
    glz::schema level{ .description = "Level needed in the craft this recipe is listed under. 255 marks a level nobody has found yet.", .minimum = 1L, .maximum = 255L };
    glz::schema subcrafts{ .description = "Level needed in other crafts, none above level." };
    glz::schema key_item{ .description = "Key item needed to attempt it." };
    glz::schema content{ .description = "Content setting that gates the recipe." };
    glz::schema crystal{ .description = "Element of the crystal. Clusters and crafted crystals of that element work too." };
    glz::schema ingredients{ .description = "Ingredients and how many of each, 8 at most in all. Item names, or the id when several items share the name." };
    glz::schema results{ .description = "What each quality tier makes." };
};

template <>
struct glz::json_schema<xi::data::datasets::synth_recipes::wire::Crafts>
{
    glz::schema woodworking{ .description = "Woodworking recipes keyed by recipe id." };
    glz::schema smithing{ .description = "Smithing recipes keyed by recipe id." };
    glz::schema goldsmithing{ .description = "Goldsmithing recipes keyed by recipe id." };
    glz::schema clothcraft{ .description = "Clothcraft recipes keyed by recipe id." };
    glz::schema leathercraft{ .description = "Leathercraft recipes keyed by recipe id." };
    glz::schema bonecraft{ .description = "Bonecraft recipes keyed by recipe id." };
    glz::schema alchemy{ .description = "Alchemy recipes keyed by recipe id." };
    glz::schema cooking{ .description = "Cooking recipes keyed by recipe id." };
};

template <>
struct glz::json_schema<xi::data::datasets::synth_recipes::wire::Document>
{
    glz::schema synth_recipes{ .description = "Recipes under the craft with the highest level requirement, the first one on a tie." };
};
