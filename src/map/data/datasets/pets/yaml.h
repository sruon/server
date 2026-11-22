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
#include "data/enums/damage_type.h"
#include "data/enums/element.h"
#include "data/yaml/enum_token.h"
#include "data/yaml/schema_annotations.h"

#include <glaze/glaze.hpp>

#include <map>
#include <optional>
#include <string>

namespace xi::data::datasets::pets::wire
{

struct Pet
{
    uint8                                          id{};
    std::string                                    name;
    uint32                                         pool{};
    std::optional<uint8>                           min_level;
    std::optional<uint8>                           max_level;
    std::optional<uint32>                          duration;
    std::optional<yaml::EnumToken<xi::Element>>    element;
    std::optional<yaml::EnumToken<xi::DamageType>> damage_type;
};

struct Document
{
    std::map<std::string, Pet> pets;

    using YamlRoot = yaml::DatasetRoot<&Document::pets>;
};

} // namespace xi::data::datasets::pets::wire

template <>
struct glz::json_schema<xi::data::datasets::pets::wire::Pet>
{
    glz::schema id{ .description = "Pet ID used by scripts and the pet packets.", .minimum = 0L, .maximum = 255L };
    glz::schema name{ .description = "Name the pet entity shows." };
    glz::schema pool{ .description = "Mob pool the pet takes its look, jobs, family and resistances from.", .minimum = 0L };
    glz::schema min_level{ .description = "Lowest level the pet can be.", .defaultValue = 1, .minimum = 1L, .maximum = 255L };
    glz::schema max_level{ .description = "Highest level the pet can be.", .defaultValue = 99, .minimum = 1L, .maximum = 255L };
    glz::schema duration{ .description = "Seconds before the pet leaves on its own. Omit when it stays until dismissed.", .defaultValue = 0, .minimum = 0L };
    glz::schema element{ .description = "Element of the pet, used by avatar perpetuation." };
    glz::schema damage_type{ .description = "Damage type of the pet's melee attacks." };
};

template <>
struct glz::json_schema<xi::data::datasets::pets::wire::Document>
{
    glz::schema pets{ .description = "Pets keyed by their canonical name." };
};
