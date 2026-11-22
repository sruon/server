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

#include "data/datasets/despoil_effects/dataset.h"

#include "data/datasets/despoil_effects/yaml.h"
#include "data/yaml/enum_token.h"
#include "data/yaml/read.h"

#include <string_view>

namespace xi::data::datasets::despoil_effects
{

auto Dataset::decode(const std::string_view text) -> Records
{
    DespoilEffects effects;
    for (const auto& [itemId, effect] : yaml::read<YamlDocument>(text).despoil_effects)
    {
        effects.emplace(itemId, yaml::resolveEnum(effect));
    }

    return effects;
}

} // namespace xi::data::datasets::despoil_effects
