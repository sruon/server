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

#include "common/enum_traits.h"
#include "data/enums/fishing_bait_flag.h"
#include "data/enums/fishing_bait_power.h"
#include "data/enums/fishing_legendary_flag.h"
#include "data/enums/fishing_nm_flag.h"
#include "data/enums/fishing_rod_flag.h"
#include "data/enums/fishing_size.h"
#include "data/enums/key_item.h"
#include "data/enums/zone.h"
#include "map/data/datasets/fishing/dataset.h"
#include "map/data/datasets/zones/fishing_areas/dataset.h"
#include "map/data/loader.h"
#include "map/data/yaml/merge.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>

namespace
{

using FishingDataset      = xi::data::datasets::fishing::Dataset;
using FishingAreasDataset = xi::data::datasets::zones::fishing_areas::Dataset;

auto slurp(const std::string& path) -> std::string
{
    std::ifstream input(path, std::ios::binary);
    return { std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };
}

template <class Records>
auto find(const Records& records, const std::string_view name) -> const auto&
{
    const auto it = std::ranges::find(records, name, &std::ranges::range_value_t<Records>::Name);
    REQUIRE(it != records.end());
    return *it;
}

auto zoneFile(const xi::ZoneId zoneId) -> xi::data::ZoneFishing
{
    const auto records = FishingAreasDataset::decode(slurp(xi::data::zoneFilePath(zoneId, FishingAreasDataset::kDataPath)));
    FishingAreasDataset::verifyZone(records, zoneId);
    return records;
}

} // namespace

TEST_CASE("fishing: catalog reads fish, rods and baits from data/fishing.yaml", "[data][fishing]")
{
    const auto catalog = FishingDataset::decode(slurp("data/fishing.yaml"));

    const auto& gugrusaurus = find(catalog.Fish, "gugrusaurus");
    CHECK(gugrusaurus.Id == 5127);
    CHECK(gugrusaurus.Size == xi::FishingSize::Large);
    CHECK(gugrusaurus.Skill == 140);
    CHECK(gugrusaurus.MinLength == 155);
    CHECK(gugrusaurus.MaxLength == 420);
    CHECK(gugrusaurus.Legendary);
    CHECK(gugrusaurus.LegendaryFlags == (xi::FishingLegendaryFlag::HalfTime | xi::FishingLegendaryFlag::NoRodTimeBonus | xi::FishingLegendaryFlag::EbisuTimeBonusOnly));
    CHECK(gugrusaurus.KeyItem == xi::KeyItem::SerpentRumors);
    CHECK(gugrusaurus.MaxHook == 1);
    CHECK(gugrusaurus.Log == 255);

    const auto& ebisu = find(catalog.Rods, "ebisu_fishing_rod");
    CHECK(ebisu.Id == 17011);
    CHECK(ebisu.Flags == xi::FishingRodFlag::LegendaryBonus);
    CHECK(ebisu.MinRank == 1);
    CHECK(ebisu.MaxRank == 30);
    CHECK(ebisu.SmallDelayBonus == 2);
    CHECK(ebisu.LargeMoveBonus == 0);
    CHECK(ebisu.BreaksInto.empty());
    CHECK(find(catalog.Rods, "bamboo_fishing_rod").BreaksInto == "broken_bamboo_fishing_rod");

    const auto& sabiki = find(catalog.Baits, "sabiki_rig");
    CHECK(sabiki.MaxHook == 3);
    const auto sardine = find(catalog.Fish, "bastore_sardine").Id;
    CHECK(std::ranges::find(sabiki.Catches, std::pair{ sardine, xi::FishingBaitPower::Common }) != sabiki.Catches.end());

    CHECK(std::ranges::is_sorted(catalog.Fish, {}, &xi::data::FishData::Id));
    CHECK(find(catalog.Fish, "abaia").Disabled);
}

TEST_CASE("fishing: zone files read areas, pools and monsters", "[data][fishing]")
{
    const auto carpenters = zoneFile(xi::ZoneId::CarpentersLanding);
    CHECK(carpenters.Difficulty == 0);

    const auto& southLanding = find(carpenters.Areas, "south_landing");
    CHECK(southLanding.Id == 1);
    CHECK(southLanding.Shape == xi::data::FishingAreaShape::Cylinder);
    CHECK(southLanding.Center == std::array{ 172.25f, -2.0f, -475.286f });
    CHECK(southLanding.Radius == 150);
    CHECK(southLanding.Height == 20);
    CHECK(std::ranges::find(southLanding.Pool, std::pair<std::string, uint16>{ "rusty_bucket", 900 }) != southLanding.Pool.end());

    const auto  bastok    = zoneFile(xi::ZoneId::BastokMarkets);
    const auto& northSide = find(bastok.Areas, "north_side");
    CHECK(northSide.Shape == xi::data::FishingAreaShape::Poly);
    CHECK(northSide.Center[1] == -6.0f);
    CHECK(northSide.Corners.size() == 9);

    const auto  alZahbi = zoneFile(xi::ZoneId::OpenSeaRouteToAlZahbi);
    const auto& orobon  = find(alZahbi.Monsters, "gugru_orobon");
    CHECK(orobon.Spawns == std::vector<uint32>{ 16965638 });
    CHECK(orobon.Area == find(alZahbi.Areas, "whole_zone").Id);
    CHECK(orobon.Nm);
    CHECK(orobon.Bait == "minnow");
    CHECK(orobon.AlternativeBait == "sinking_minnow");
    CHECK(orobon.Delay == 10);
    CHECK(orobon.Rarity == 1000);
}

TEST_CASE("fishing: every zone file names fish, baits and spawns the catalog knows", "[data][fishing]")
{
    const auto            catalog = FishingDataset::decode(slurp("data/fishing.yaml"));
    std::set<std::string> fish;
    std::set<std::string> baits;
    for (const auto& entry : catalog.Fish)
    {
        fish.insert(entry.Name);
    }

    for (const auto& entry : catalog.Baits)
    {
        baits.insert(entry.Name);
    }

    auto zones = 0;
    for (const auto& [key, zoneId] : xi::data::EnumTraits<xi::ZoneId>::kEntries)
    {
        if (!std::filesystem::exists(xi::data::zoneFilePath(zoneId, FishingAreasDataset::kDataPath)))
        {
            continue;
        }

        ++zones;
        const auto zone = zoneFile(zoneId);
        for (const auto& area : zone.Areas)
        {
            for (const auto& [name, rarity] : area.Pool)
            {
                INFO(key << " " << area.Name << " " << name);
                CHECK(fish.contains(name));
            }
        }

        for (const auto& monster : zone.Monsters)
        {
            INFO(key << " " << monster.Name);
            CHECK((monster.Bait.empty() || baits.contains(monster.Bait)));
            CHECK((monster.AlternativeBait.empty() || baits.contains(monster.AlternativeBait)));
        }
    }

    CHECK(zones == 99);
}

TEST_CASE("fishing: a module patch changes a rod and adds a catch to a pool", "[data][fishing]")
{
    const auto catalogPatch = std::array{ std::string{ "fishing:\n  rods:\n    halcyon_rod:\n      time: 50\n  baits:\n    ball_of_insect_paste:\n      catches:\n        moat_carp: rare\n" } };
    const auto catalog      = FishingDataset::decode(xi::data::mergeYaml(slurp("data/fishing.yaml"), catalogPatch));

    CHECK(find(catalog.Rods, "halcyon_rod").Time == 50);
    CHECK(find(catalog.Rods, "halcyon_rod").BreaksInto == "broken_halcyon_fishing_rod");
    const auto  moatCarp    = find(catalog.Fish, "moat_carp").Id;
    const auto& insectPaste = find(catalog.Baits, "ball_of_insect_paste");
    CHECK(std::ranges::find(insectPaste.Catches, std::pair{ moatCarp, xi::FishingBaitPower::Rare }) != insectPaste.Catches.end());

    const auto zonePatch = std::array{ std::string{ "difficulty: 4\nareas:\n  south_landing:\n    pool:\n      moat_carp: 250\n" } };
    const auto zone      = FishingAreasDataset::decode(xi::data::mergeYaml(slurp(xi::data::zoneFilePath(xi::ZoneId::CarpentersLanding, FishingAreasDataset::kDataPath)), zonePatch));

    CHECK(zone.Difficulty == 4);
    const auto& southLanding = find(zone.Areas, "south_landing");
    CHECK(std::ranges::find(southLanding.Pool, std::pair<std::string, uint16>{ "moat_carp", 250 }) != southLanding.Pool.end());
    CHECK(std::ranges::find(southLanding.Pool, std::pair<std::string, uint16>{ "rusty_bucket", 900 }) != southLanding.Pool.end());
    CHECK(southLanding.Shape == xi::data::FishingAreaShape::Cylinder);
}

TEST_CASE("fishing: bad zone files are rejected", "[data][fishing]")
{
    CHECK_THROWS_AS(FishingAreasDataset::decode("areas:\n  pond:\n    id: 1\n    cylinder: {center: [0, 0, 0], radius: 5, height: 5}\n    poly: {floor: 0, height: 5, corners: [[0, 0], [1, 0], [0, 1]]}\n"), std::runtime_error);
    CHECK_THROWS_AS(FishingAreasDataset::decode("areas:\n  pond:\n    id: 1\nmonsters:\n  crab:\n    spawns: [16785409]\n    area: lake\n    level: 10\n    difficulty: 15\n"), std::runtime_error);

    const auto wrongZone = FishingAreasDataset::decode("monsters:\n  crab:\n    spawns: [16785409]\n    level: 10\n    difficulty: 15\n");
    CHECK_THROWS_AS(FishingAreasDataset::verifyZone(wrongZone, xi::ZoneId::WestRonfaure), std::runtime_error);
}
