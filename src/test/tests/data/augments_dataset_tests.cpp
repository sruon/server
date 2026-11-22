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

#include "data/enums/mod.h"
#include "data/enums/pet_mod_type.h"
#include "map/data/datasets/augments/dataset.h"
#include "map/data/yaml/merge.h"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <fstream>
#include <iterator>
#include <string>

namespace
{

using AugmentsDataset = xi::data::datasets::augments::Dataset;

auto slurp(const std::string& path) -> std::string
{
    std::ifstream input(path, std::ios::binary);
    return { std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };
}

auto find(const xi::data::Augments& augments, const uint16 id) -> const xi::data::AugmentData&
{
    const auto it = augments.find(id);
    REQUIRE(it != augments.end());
    return it->second;
}

} // namespace

TEST_CASE("augments: mods, pets and multipliers read from data/augments.yaml", "[data][augment]")
{
    const auto augments = AugmentsDataset::decode(slurp("data/augments.yaml"));

    const auto& hpMp = find(augments, 17);
    CHECK(hpMp.Mods.size() == 2);
    CHECK(hpMp.Mods.at(xi::Mod::HP) == 1);
    CHECK(hpMp.Mods.at(xi::Mod::MP) == 1);
    CHECK_FALSE(hpMp.Pet.has_value());
    CHECK(hpMp.Multiplier == 1);

    const auto& haste = find(augments, 49);
    CHECK(haste.Mods.at(xi::Mod::HASTE_GEAR) == 1);
    CHECK(haste.Multiplier == 100);

    CHECK(find(augments, 96).Pet == xi::PetModType::All);
    CHECK(find(augments, 120).Pet == xi::PetModType::Avatar);

    CHECK_FALSE(augments.contains(72));
}

TEST_CASE("augments: a module patch changes, adds and removes augments", "[data][augment]")
{
    const auto modules  = std::array{ std::string{ "augments:\n  1: {mods: {hp: 2}}\n  17: {mods: {mp: null}}\n  49: null\n  72: {mods: {exp_bonus: 1}}\n" } };
    const auto augments = AugmentsDataset::decode(xi::data::mergeYaml(slurp("data/augments.yaml"), modules));

    CHECK(find(augments, 1).Mods.at(xi::Mod::HP) == 2);

    const auto& hpMp = find(augments, 17);
    CHECK(hpMp.Mods.size() == 1);
    CHECK(hpMp.Mods.contains(xi::Mod::HP));

    CHECK_FALSE(augments.contains(49));
    CHECK(find(augments, 72).Mods.at(xi::Mod::EXP_BONUS) == 1);
}

TEST_CASE("augments: IDs outside 1 to 2047 and empty mods are rejected", "[data][augment]")
{
    CHECK_THROWS(AugmentsDataset::decode("augments:\n  2048: {mods: {hp: 1}}\n"));
    CHECK_THROWS(AugmentsDataset::decode("augments:\n  0: {mods: {hp: 1}}\n"));
    CHECK_THROWS(AugmentsDataset::decode("augments:\n  1: {mods: {}}\n"));
    CHECK_THROWS(AugmentsDataset::decode("augments:\n  1: {mods: {none: 1}}\n"));
    CHECK_THROWS(AugmentsDataset::decode("augments:\n  1: {mods: {hp: 1}, multiplier: 0}\n"));
}
