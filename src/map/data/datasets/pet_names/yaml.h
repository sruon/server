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

namespace xi::data::datasets::pet_names::wire
{

struct Document
{
    std::map<uint16, std::string> pet_names;

    using YamlRoot = yaml::DatasetRoot<&Document::pet_names>;
};

} // namespace xi::data::datasets::pet_names::wire

template <>
struct glz::json_schema<xi::data::datasets::pet_names::wire::Document>
{
    glz::schema pet_names{ .description = "Pet names keyed by the name id the client sends and shows." };
};
