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
#include "data/enums/element.h"
#include "data/enums/flowerpot_plant.h"
#include "data/yaml/enum_keyed_map.h"
#include "data/yaml/schema_annotations.h"

#include <glaze/glaze.hpp>

#include <array>
#include <vector>

namespace xi::data::datasets::gardening_results::wire
{

struct Result
{
    uint16               item{};
    std::array<uint8, 2> quantity{};
    uint8                weight{};
};

using Results = std::vector<Result>;

template <class Value>
using ByCrystal = yaml::EnumKeyedMap<xi::Element, Value>;

struct Tables
{
    yaml::EnumKeyedMap<xi::FlowerpotPlant, ByCrystal<Results>>            plants;
    yaml::EnumKeyedMap<xi::FlowerpotPlant, ByCrystal<ByCrystal<Results>>> trees;
};

struct Document
{
    Tables gardening_results;

    using YamlRoot = yaml::DatasetRoot<&Document::gardening_results>;
};

} // namespace xi::data::datasets::gardening_results::wire

template <>
struct glz::meta<xi::data::datasets::gardening_results::wire::Result>
{
    static constexpr std::string_view name = "gardening_result";
};

template <>
struct glz::json_schema<xi::data::datasets::gardening_results::wire::Result>
{
    glz::schema item{ .description = "Item id harvested.", .minimum = 1L, .maximum = 65535L };
    glz::schema quantity{ .description = "Fewest and most harvested, as [min, max]." };
    glz::schema weight{ .description = "Width of the harvest strength band this result covers.", .minimum = 1L, .maximum = 100L };
};

template <>
struct glz::json_schema<xi::data::datasets::gardening_results::wire::Tables>
{
    glz::schema plants{ .description = "Plants fed one crystal, keyed by plant then crystal." };
    glz::schema trees{ .description = "Plants fed two crystals, keyed by plant, then one crystal, then the other. A pair applies in both feed orders unless the reverse is listed too." };
};

template <>
struct glz::json_schema<xi::data::datasets::gardening_results::wire::Document>
{
    glz::schema gardening_results{ .description = "What a flowerpot yields at harvest. Results share out the harvest strength in list order." };
};
