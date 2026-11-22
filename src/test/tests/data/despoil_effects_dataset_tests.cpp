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

#include "map/data/datasets/despoil_effects/dataset.h"

#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <iterator>
#include <string>

namespace
{

using DespoilEffectsDataset = xi::data::datasets::despoil_effects::Dataset;

auto slurp(const std::string& path) -> std::string
{
    std::ifstream input(path, std::ios::binary);
    return { std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };
}

} // namespace

TEST_CASE("despoil effects: read from data/despoil_effects.yaml", "[data][despoil_effects]")
{
    const auto effects = DespoilEffectsDataset::decode(slurp("data/despoil_effects.yaml"));

    CHECK(effects.size() == 7);
    CHECK(effects.at(644) == xi::StatusEffect::DefenseDown);
    CHECK(effects.at(4400) == xi::StatusEffect::Slow);
    CHECK_FALSE(effects.contains(4401));
}

TEST_CASE("despoil effects: unknown effects are rejected", "[data][despoil_effects]")
{
    CHECK_THROWS(DespoilEffectsDataset::decode(R"(
despoil_effects:
  644: not_an_effect
)"));
}
