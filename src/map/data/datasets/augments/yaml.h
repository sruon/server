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
#include "data/enums/mod.h"
#include "data/enums/pet_mod_type.h"
#include "data/yaml/enum_keyed_map.h"
#include "data/yaml/enum_token.h"
#include "data/yaml/schema_annotations.h"

#include <glaze/glaze.hpp>

#include <map>
#include <optional>

namespace xi::data::datasets::augments::wire
{

struct Augment
{
    std::optional<yaml::EnumToken<xi::PetModType>> pet;
    yaml::EnumKeyedMap<xi::Mod, int16>             mods;
    std::optional<uint16>                          multiplier;
};

struct Document
{
    std::map<uint16, Augment> augments;

    using YamlRoot = yaml::DatasetRoot<&Document::augments>;
};

} // namespace xi::data::datasets::augments::wire

template <>
struct glz::json_schema<xi::data::datasets::augments::wire::Augment>
{
    glz::schema pet{ .description = "Pets the mods go to. Omit when they go to the wearer." };
    glz::schema mods{ .description = "Mod amounts at augment value 0, keyed by mod name. Each point of augment value adds one more in the same direction." };
    glz::schema multiplier{ .description = "Scales every mod amount after the augment value is added.", .defaultValue = 1, .minimum = 1L, .maximum = 255L };
};

template <>
struct glz::json_schema<xi::data::datasets::augments::wire::Document>
{
    glz::schema augments{ .description = "Standard augments keyed by augment ID, from 1 to 2047. IDs not listed grant no mods." };
};
