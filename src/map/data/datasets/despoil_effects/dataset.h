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

#include <map>
#include <string_view>

namespace xi::data
{

using DespoilEffects = std::map<uint16, xi::StatusEffect>; // keyed by item id

} // namespace xi::data

namespace xi::data::datasets::despoil_effects::wire
{

struct Document;

}

namespace xi::data::datasets::despoil_effects
{

struct Dataset
{
    using Records      = DespoilEffects;
    using YamlDocument = wire::Document;

    static constexpr std::string_view kDataPath{ "despoil_effects" };
    static constexpr std::string_view kTitle{ "Despoil Effects" };
    static constexpr std::string_view kDescription{ "The effect Despoil lands on the target, keyed by the item stolen." };

    static auto decode(std::string_view text) -> Records;
};

} // namespace xi::data::datasets::despoil_effects
