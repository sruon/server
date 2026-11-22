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

#include "map/data/datasets/zones/instances/dataset.h"
#include "map/data/datasets/zones/mobs/dataset.h"
#include "map/data/datasets/zones/npcs/dataset.h"
#include "map/data/loader.h"
#include "map/data/yaml/merge.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{

using InstancesDataset = xi::data::datasets::zones::instances::Dataset;
using MobsDataset      = xi::data::datasets::zones::mobs::Dataset;
using NpcsDataset      = xi::data::datasets::zones::npcs::Dataset;

constexpr auto kInstances = R"(
instances:
  golden_salvage:
    id:         5500
    entrance:   arrapago_reef
    time_limit: 30
    entry:      [386.000, -12.000, 17.000, 46]
  path_of_darkness:
    id:       7700
    entrance: alzadaal_undersea_ruins
    entry:    [500.000, 0.000, -572.000, 192]
    music:
      day:         143
      battle_solo: 143
)";

auto find(const xi::data::Instances& records, const std::string& name) -> const xi::data::InstanceData&
{
    const auto found = std::ranges::find(records, name, &xi::data::InstanceData::Name);
    REQUIRE(found != records.end());
    return *found;
}

} // namespace

TEST_CASE("instances: an instance keeps its entry and limits", "[data][instance]")
{
    const auto records = InstancesDataset::decode(kInstances);
    REQUIRE(records.size() == 2);

    const auto& salvage = find(records, "golden_salvage");
    CHECK(salvage.Id == 5500);
    CHECK(salvage.Entrance == xi::ZoneId::ArrapagoReef);
    CHECK(salvage.TimeLimit == std::chrono::minutes(30));
    CHECK(salvage.Entry.x == 386.0f);
    CHECK(salvage.Entry.z == 17.0f);
    CHECK(salvage.Entry.rotation == 46);
    CHECK(salvage.Overlay == 0);
    CHECK_FALSE(salvage.Music.Day.has_value());
}

TEST_CASE("instances: music overrides only what it names", "[data][instance]")
{
    const auto  records  = InstancesDataset::decode(kInstances);
    const auto& darkness = find(records, "path_of_darkness");

    CHECK(darkness.TimeLimit == std::chrono::minutes(0));
    CHECK(darkness.Music.Day == 143);
    CHECK(darkness.Music.BattleSolo == 143);
    CHECK_FALSE(darkness.Music.Night.has_value());
    CHECK_FALSE(darkness.Music.BattleParty.has_value());
}

TEST_CASE("instances: two instances cannot share an id", "[data][instance]")
{
    constexpr auto twice = R"(
instances:
  first:
    id:       5500
    entrance: arrapago_reef
    entry:    [0, 0, 0]
  second:
    id:       5500
    entrance: arrapago_reef
    entry:    [0, 0, 0]
)";

    REQUIRE_THROWS_AS(InstancesDataset::decode(twice), std::runtime_error);
}

TEST_CASE("instances: a module patch retunes one instance and leaves the rest", "[data][instance]")
{
    const std::vector<std::string> modules{ R"(
instances:
  golden_salvage:
    time_limit: 45
    music:
      day: 187
)" };

    const auto records = InstancesDataset::decode(xi::data::mergeYaml(kInstances, modules));
    REQUIRE(records.size() == 2);

    const auto& salvage = find(records, "golden_salvage");
    CHECK(salvage.TimeLimit == std::chrono::minutes(45));
    CHECK(salvage.Music.Day == 187);
    CHECK(salvage.Entry.x == 386.0f);
    CHECK(find(records, "path_of_darkness").TimeLimit == std::chrono::minutes(0));
}

TEST_CASE("instances: spawns and npcs name the instances they join", "[data][instance]")
{
    constexpr auto mobs = R"(
templates:
  Percipient_Fish:
    id:      1
    species: crab
spawns:
  17002497:
    instance: golden_salvage
    template: Percipient_Fish
    at:       [260.272, -2.000, 24.269]
  17002498:
    instance: [golden_salvage, lamia_no_13]
    template: Percipient_Fish
    at:       [309.655, -7.136, 56.875]
  17002499:
    template: Percipient_Fish
    at:       [300.966, -3.209, 67.613]
)";

    const auto mobRecords = MobsDataset::decode(mobs);
    REQUIRE(mobRecords.Spawns.size() == 3);
    CHECK(mobRecords.Spawns[0].Instances == std::vector<std::string>{ "golden_salvage" });
    CHECK(mobRecords.Spawns[1].Instances == std::vector<std::string>{ "golden_salvage", "lamia_no_13" });
    CHECK(mobRecords.Spawns[2].Instances.empty());

    constexpr auto npcs = R"(
npcs:
  17002654:
    instance: [golden_salvage, lamia_no_13, extermination]
    script:   Ancient_Lockbox
    render:
      look:
        type:  standard
        model: 50
)";

    const auto npcRecords = NpcsDataset::decode(npcs);
    REQUIRE(npcRecords.size() == 1);
    CHECK(npcRecords.front().Instances.size() == 3);
}

TEST_CASE("instances: every tagged entity in Ilrusi Atoll names an instance the zone declares", "[data][instance]")
{
    const auto instances = xi::data::loadZoneFile<InstancesDataset>(xi::ZoneId::IlrusiAtoll);
    const auto mobs      = xi::data::loadZoneFile<MobsDataset>(xi::ZoneId::IlrusiAtoll);
    const auto npcs      = xi::data::loadZoneFile<NpcsDataset>(xi::ZoneId::IlrusiAtoll);
    REQUIRE(instances.has_value());
    REQUIRE(mobs.has_value());
    REQUIRE(npcs.has_value());

    const auto declared = [&](const std::string& name)
    {
        return std::ranges::find(*instances, name, &xi::data::InstanceData::Name) != instances->end();
    };

    size_t tagged = 0;
    for (const auto& spawn : mobs->Spawns)
    {
        for (const auto& name : spawn.Instances)
        {
            CHECK(declared(name));
            ++tagged;
        }
    }

    for (const auto& npc : *npcs)
    {
        for (const auto& name : npc.Instances)
        {
            CHECK(declared(name));
            ++tagged;
        }
    }

    CHECK(tagged > 0);
}
