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
#include "data/enums/status_effect.h"
#include "data/yaml/enum_token.h"
#include "data/yaml/schema_annotations.h"

#include <glaze/glaze.hpp>

#include <map>

namespace xi::data::datasets::despoil_effects::wire
{

struct Document
{
    std::map<uint16, yaml::EnumToken<xi::StatusEffect>> despoil_effects;

    using YamlRoot = yaml::DatasetRoot<&Document::despoil_effects>;
};

} // namespace xi::data::datasets::despoil_effects::wire

template <>
struct glz::json_schema<xi::data::datasets::despoil_effects::wire::Document>
{
    glz::schema despoil_effects{ .description = "The effect Despoil lands on the target, keyed by the item id stolen." };
};
