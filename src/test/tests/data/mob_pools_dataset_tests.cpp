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

#include "data/enums/job.h"
#include "data/enums/mob_type.h"
#include "data/enums/mod.h"
#include "data/enums/skill_type.h"
#include "data/enums/species.h"
#include "map/data/datasets/mob_pools/dataset.h"
#include "map/data/yaml/merge.h"
#include "map/utils/mobutils.h"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <fstream>
#include <iterator>
#include <string>

namespace
{

using MobPoolsDataset = xi::data::datasets::mob_pools::Dataset;

auto slurp(const std::string& path) -> std::string
{
    std::ifstream input(path, std::ios::binary);
    return { std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };
}

} // namespace

TEST_CASE("mob_pools: pools read from data/mob_pools.yaml", "[data][mob_pools]")
{
    const auto pools = MobPoolsDataset::decode(slurp("data/mob_pools.yaml"));

    REQUIRE(pools.contains(44));
    const auto& adamantoise = pools.at(44);
    CHECK(adamantoise.Name == "Adamantoise");
    CHECK(adamantoise.DisplayName == "Adamantoise");
    CHECK(adamantoise.Species == xi::Species::Adamantoise);
    CHECK(adamantoise.Type == xi::MobType::Notorious);
    CHECK(adamantoise.SkillList == 2);
    CHECK(adamantoise.Attributes.MainJob == xi::Job::PLD);
    CHECK(adamantoise.Attributes.CombatSkill == xi::SkillType::Scythe);
    CHECK(adamantoise.Attributes.EntityFlags == 1153u);
    CHECK(adamantoise.Attributes.Aggressive == true);
    CHECK(adamantoise.Attributes.Mods.at(xi::Mod::REGAIN) == 150);
    REQUIRE(adamantoise.Attributes.Look.has_value());
    CHECK((*adamantoise.Attributes.Look)[0] == 0);
    CHECK((*adamantoise.Attributes.Look)[1] == 400);

    REQUIRE(pools.contains(5896));
    const auto& shantotto = pools.at(5896);
    CHECK(shantotto.Name == "shantotto");
    CHECK(shantotto.DisplayName == "Shantotto");
    CHECK(shantotto.SpellList == 308);
    CHECK(shantotto.Attributes.SubJob == xi::Job::NONE);
    CHECK(shantotto.Attributes.DamageMultiplier == 40);
    CHECK_FALSE(shantotto.Attributes.Delay.has_value());

    CHECK(pools.at(831).Name == "Crawler_831");
    CHECK(pools.at(831).DisplayName == "Crawler");
}

TEST_CASE("mob_pools: resists only restate what differs from the species", "[data][mob_pools]")
{
    mobutils::LoadSpeciesData();
    const auto pools = MobPoolsDataset::decode(slurp("data/mob_pools.yaml"));

    const auto resolved = mobutils::ResolvePoolAttributes(pools.at(44));
    CHECK(resolved.Resists.at(xi::Mod::STUN_RES_RANK) == 0);
    CHECK(resolved.Resists.at(xi::Mod::SLOW_RES_RANK) == 11);
}

TEST_CASE("mob_pools: a module patches a pool by name", "[data][mob_pools]")
{
    const auto modules = std::array{ std::string{ "mob_pools:\n"
                                                  "  Adamantoise:\n"
                                                  "    display_name: Ancient_Adamantoise\n"
                                                  "    attributes:\n"
                                                  "      mods:\n"
                                                  "        regain: 50\n"
                                                  "        hpp:    25\n" } };
    const auto pools   = MobPoolsDataset::decode(xi::data::mergeYaml(slurp("data/mob_pools.yaml"), modules));

    const auto& adamantoise = pools.at(44);
    CHECK(adamantoise.DisplayName == "Ancient_Adamantoise");
    CHECK(adamantoise.Attributes.Mods.at(xi::Mod::REGAIN) == 50);
    CHECK(adamantoise.Attributes.Mods.at(xi::Mod::HPP) == 25);
    CHECK(adamantoise.Attributes.MainJob == xi::Job::PLD);
    CHECK(adamantoise.Attributes.Look.has_value());
}

TEST_CASE("mob_pools: two pools cannot share an id", "[data][mob_pools]")
{
    const auto modules = std::array{ std::string{ "mob_pools:\n  Copy_Of_Adamantoise:\n    id:      44\n    species: adamantoise\n" } };

    CHECK_THROWS(MobPoolsDataset::decode(xi::data::mergeYaml(slurp("data/mob_pools.yaml"), modules)));
}
