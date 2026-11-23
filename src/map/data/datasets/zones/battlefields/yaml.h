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
#include "data/yaml/schema_annotations.h"

#include <glaze/glaze.hpp>

#include <map>
#include <string>

namespace xi::data::datasets::zones::battlefields::wire
{

struct Battlefield
{
    uint8 index{};
};

struct Document
{
    std::map<std::string, Battlefield> battlefields;

    using YamlRoot = yaml::DatasetRoot<&Document::battlefields>;
};

} // namespace xi::data::datasets::zones::battlefields::wire

template <>
struct glz::json_schema<xi::data::datasets::zones::battlefields::wire::Battlefield>
{
    glz::schema index{ .description = "Position in the zone's entry menu, sent to the client in the entry and win events.", .minimum = 0L, .maximum = 31L };
};

template <>
struct glz::json_schema<xi::data::datasets::zones::battlefields::wire::Document>
{
    glz::schema battlefields{ .description = "Battlefields the zone hosts, keyed by their canonical name in index order." };
};
