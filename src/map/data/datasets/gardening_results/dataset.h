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

#include <map>
#include <string_view>
#include <tuple>
#include <vector>

namespace xi::data
{

struct GardeningResult
{
    uint16 ItemId{};
    uint8  MinQuantity{};
    uint8  MaxQuantity{};
    uint8  Weight{};
};

// plant, first crystal fed, second crystal fed; plants fed once hold none first
using GardeningKey     = std::tuple<xi::FlowerpotPlant, xi::Element, xi::Element>;
using GardeningResults = std::map<GardeningKey, std::vector<GardeningResult>>;

} // namespace xi::data

namespace xi::data::datasets::gardening_results::wire
{

struct Document;

}

namespace xi::data::datasets::gardening_results
{

struct Dataset
{
    using Records      = GardeningResults;
    using YamlDocument = wire::Document;

    static constexpr std::string_view kDataPath{ "gardening_results" };
    static constexpr std::string_view kTitle{ "Gardening Results" };
    static constexpr std::string_view kDescription{ "What a flowerpot yields at harvest, by plant and the crystals it was fed." };

    static auto decode(std::string_view text) -> Records;
};

} // namespace xi::data::datasets::gardening_results
