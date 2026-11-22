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

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace xi::data
{

struct BlueTraitTierData
{
    uint8                                  Rank{};
    uint8                                  Points{};
    uint16                                 Trait{};
    std::vector<std::pair<xi::Mod, int16>> Mods{};
    bool                                   JobPointsOnly{};
};

struct BlueTraitCategoryData
{
    uint8                          Id{};
    std::string                    Name{};
    std::vector<BlueTraitTierData> Tiers{}; // rank order
};

using BlueTraits = std::vector<BlueTraitCategoryData>; // ordered by id

} // namespace xi::data

namespace xi::data::datasets::blue_traits::wire
{

struct Document;

}

namespace xi::data::datasets::blue_traits
{

struct Dataset
{
    using Records      = BlueTraits;
    using YamlDocument = wire::Document;

    static constexpr std::string_view kDataPath{ "blue_traits" };
    static constexpr std::string_view kTitle{ "Blue Traits" };
    static constexpr std::string_view kDescription{ "Traits granted by the blue magic spells a blue mage sets." };

    static auto decode(std::string_view text) -> Records;
};

} // namespace xi::data::datasets::blue_traits
