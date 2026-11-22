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
#include "data/yaml/enum_keyed_map.h"
#include "data/yaml/schema_annotations.h"

#include <glaze/glaze.hpp>

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace xi::data::datasets::blue_traits::wire
{

struct Tier
{
    uint8                              rank{};
    uint8                              points{};
    std::optional<uint16>              trait;
    yaml::EnumKeyedMap<xi::Mod, int16> mods;
    std::optional<bool>                job_points_only;
};

struct Category
{
    uint8             id{};
    uint16            trait{};
    std::vector<Tier> tiers;
};

struct Document
{
    std::map<std::string, Category> blue_traits;

    using YamlRoot = yaml::DatasetRoot<&Document::blue_traits>;
};

} // namespace xi::data::datasets::blue_traits::wire

template <>
struct glz::json_schema<xi::data::datasets::blue_traits::wire::Tier>
{
    glz::schema rank{ .description = "Trait rank this tier grants.", .minimum = 0L, .maximum = 255L };
    glz::schema points{ .description = "Category weight the set spells need for this tier.", .minimum = 1L, .maximum = 255L };
    glz::schema trait{ .description = "Trait ID for this tier when it differs from the category's." };
    glz::schema mods{ .description = "Modifiers the tier grants." };
    glz::schema job_points_only{ .description = "Only granted through job point gifts, never by setting spells.", .defaultValue = false };
};

template <>
struct glz::json_schema<xi::data::datasets::blue_traits::wire::Category>
{
    glz::schema id{ .description = "Internal category ID.", .minimum = 1L, .maximum = 255L };
    glz::schema trait{ .description = "Trait ID the tiers grant." };
    glz::schema tiers{ .description = "Tiers in rank order.", .minItems = 1UL };
};

template <>
struct glz::json_schema<xi::data::datasets::blue_traits::wire::Document>
{
    glz::schema blue_traits{ .description = "Blue magic trait categories keyed by the trait they open with." };
};
