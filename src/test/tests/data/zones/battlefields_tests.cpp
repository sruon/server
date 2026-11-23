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

#include "data/enums/battlefield.h"
#include "map/data/datasets/zones/battlefields/dataset.h"
#include "map/data/loader.h"
#include "map/data/yaml/merge.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <string>
#include <vector>

namespace
{

using BattlefieldsDataset = xi::data::datasets::zones::battlefields::Dataset;

auto horlaisPeakFile() -> std::string
{
    return xi::data::loadMergedYaml(xi::data::zoneFilePath(xi::ZoneId::HorlaisPeak, BattlefieldsDataset::kDataPath), {});
}

auto find(const xi::data::Battlefields& battlefields, const xi::ZoneId zone, const xi::BattlefieldId id) -> const xi::data::BattlefieldData*
{
    const auto it = std::ranges::find_if(battlefields.Entries,
                                         [&](const xi::data::BattlefieldData& entry)
                                         {
                                             return xi::data::battlefieldId(zone, entry.Index) == id;
                                         });
    if (it == battlefields.Entries.end())
    {
        return nullptr;
    }

    return &*it;
}

} // namespace

TEST_CASE("battlefields: an id is the zone and the entry menu index", "[data][battlefield]")
{
    CHECK(xi::data::battlefieldId(xi::ZoneId::HorlaisPeak, 0) == xi::BattlefieldId::Rank2Mission1);
    CHECK(xi::data::battlefieldId(xi::ZoneId::MonarchLinn, 0) == xi::BattlefieldId::AncientVows);
}

TEST_CASE("battlefields: a zone lists the battlefields it hosts", "[data][battlefield]")
{
    const auto battlefields = xi::data::loadZoneFile<BattlefieldsDataset>(xi::ZoneId::HorlaisPeak);
    REQUIRE(battlefields.has_value());
    CHECK(std::ranges::is_sorted(battlefields->Entries, {}, &xi::data::BattlefieldData::Index));

    const auto* tailsOfWoe = find(*battlefields, xi::ZoneId::HorlaisPeak, xi::BattlefieldId::TailsOfWoe);
    REQUIRE(tailsOfWoe != nullptr);
    CHECK(tailsOfWoe->Name == "tails_of_woe");
    CHECK(tailsOfWoe->Index == 1);
}

TEST_CASE("battlefields: limits and rules fall back to the framework defaults", "[data][battlefield]")
{
    const auto  modules      = std::array{ std::string{ "battlefields:\n  plain_trial: {index: 30}\n" } };
    const auto  battlefields = BattlefieldsDataset::decode(xi::data::mergeYaml(horlaisPeakFile(), modules));
    const auto* plain        = find(battlefields, xi::ZoneId::HorlaisPeak, xi::data::battlefieldId(xi::ZoneId::HorlaisPeak, 30));
    REQUIRE(plain != nullptr);

    CHECK(plain->TimeLimit == std::chrono::minutes(30));
    CHECK(plain->MaxPlayers == 6);
    CHECK(plain->LevelCap == 0);
    CHECK_FALSE(plain->CapAtMaxLevel);
    CHECK(plain->CanLoseExp);
    CHECK_FALSE(plain->AllowTrusts);
    CHECK_FALSE(plain->IsMission);
    CHECK(plain->AllowSubjob);
    CHECK_FALSE(plain->CleanupDebuffs);
    CHECK(plain->DelayToExit == std::chrono::seconds(5));
    CHECK(plain->HasWipeGrace);
    CHECK(plain->ShowTimer);
}

TEST_CASE("battlefields: a zone file sets limits and rules", "[data][battlefield]")
{
    const auto battlefields = xi::data::loadZoneFile<BattlefieldsDataset>(xi::ZoneId::HorlaisPeak);
    REQUIRE(battlefields.has_value());

    const auto* rankTwo = find(*battlefields, xi::ZoneId::HorlaisPeak, xi::BattlefieldId::Rank2Mission1);
    REQUIRE(rankTwo != nullptr);
    CHECK(rankTwo->TimeLimit == std::chrono::minutes(15));
    CHECK(rankTwo->CapAtMaxLevel);
    CHECK_FALSE(rankTwo->CanLoseExp);
    CHECK(rankTwo->AllowTrusts);
    CHECK(rankTwo->IsMission);

    const auto* tailsOfWoe = find(*battlefields, xi::ZoneId::HorlaisPeak, xi::BattlefieldId::TailsOfWoe);
    REQUIRE(tailsOfWoe != nullptr);
    CHECK(tailsOfWoe->LevelCap == 40);
    CHECK_FALSE(tailsOfWoe->CapAtMaxLevel);

    const auto* amanTrove = find(*battlefields, xi::ZoneId::HorlaisPeak, xi::BattlefieldId::AmanTroveMarsHorlaisPeak);
    REQUIRE(amanTrove != nullptr);
    CHECK(amanTrove->DelayToExit == std::chrono::seconds(4));
}

TEST_CASE("battlefields: a module changes a battlefield's limits", "[data][battlefield]")
{
    const auto  modules      = std::array{ std::string{ "battlefields:\n  tails_of_woe:\n    level_cap: max\n    time_limit: 45\n" } };
    const auto  battlefields = BattlefieldsDataset::decode(xi::data::mergeYaml(horlaisPeakFile(), modules));
    const auto* tailsOfWoe   = find(battlefields, xi::ZoneId::HorlaisPeak, xi::BattlefieldId::TailsOfWoe);
    REQUIRE(tailsOfWoe != nullptr);

    CHECK(tailsOfWoe->CapAtMaxLevel);
    CHECK(tailsOfWoe->TimeLimit == std::chrono::minutes(45));
}

TEST_CASE("battlefields: a level cap is a level or max", "[data][battlefield]")
{
    const auto modules = std::array{ std::string{ "battlefields:\n  tails_of_woe:\n    level_cap: highest\n" } };

    CHECK_THROWS(BattlefieldsDataset::decode(xi::data::mergeYaml(horlaisPeakFile(), modules)));
}

TEST_CASE("battlefields: a zone has three arenas unless its file says otherwise", "[data][battlefield]")
{
    const auto horlaisPeak = xi::data::loadZoneFile<BattlefieldsDataset>(xi::ZoneId::HorlaisPeak);
    REQUIRE(horlaisPeak.has_value());
    CHECK(horlaisPeak->MaxAreas == 3);

    const auto ghelsbaOutpost = xi::data::loadZoneFile<BattlefieldsDataset>(xi::ZoneId::GhelsbaOutpost);
    REQUIRE(ghelsbaOutpost.has_value());
    CHECK(ghelsbaOutpost->MaxAreas == 1);

    const auto* saveTheChildren = find(*ghelsbaOutpost, xi::ZoneId::GhelsbaOutpost, xi::BattlefieldId::SaveTheChildren);
    REQUIRE(saveTheChildren != nullptr);
    CHECK(saveTheChildren->Area == 1);
    CHECK(saveTheChildren->EntryNpcs == std::vector<std::string>{ "Hut_Door" });
    CHECK(saveTheChildren->ExitNpcs.empty());
}

TEST_CASE("battlefields: a zone file names entry and exit npcs, rewards and trust rules", "[data][battlefield]")
{
    const auto battlefields = xi::data::loadZoneFile<BattlefieldsDataset>(xi::ZoneId::HorlaisPeak);
    REQUIRE(battlefields.has_value());

    const auto* shatteringStars = find(*battlefields, xi::ZoneId::HorlaisPeak, xi::BattlefieldId::ShatteringStarsWar);
    REQUIRE(shatteringStars != nullptr);
    CHECK(shatteringStars->EntryNpcs == std::vector<std::string>{ "BC_Entrance" });
    CHECK(shatteringStars->ExitNpcs == std::vector<std::string>{ "Burning_Circle" });
    CHECK(shatteringStars->RhapsodyTrusts);
    CHECK_FALSE(shatteringStars->Area.has_value());
    CHECK(shatteringStars->AllowedAreas.empty());

    const auto* tailsOfWoe = find(*battlefields, xi::ZoneId::HorlaisPeak, xi::BattlefieldId::TailsOfWoe);
    REQUIRE(tailsOfWoe != nullptr);
    CHECK_FALSE(tailsOfWoe->RhapsodyTrusts);
    CHECK_FALSE(tailsOfWoe->GrantXP.has_value());
    CHECK_FALSE(tailsOfWoe->Experimental);
}

TEST_CASE("battlefields: npcs fall back to the zone's and a battlefield can override them", "[data][battlefield]")
{
    const auto laloff = xi::data::loadZoneFile<BattlefieldsDataset>(xi::ZoneId::LaloffAmphitheater);
    REQUIRE(laloff.has_value());

    const auto* arkAngels = find(*laloff, xi::ZoneId::LaloffAmphitheater, xi::BattlefieldId::ArkAngels1);
    REQUIRE(arkAngels != nullptr);
    CHECK(arkAngels->EntryNpcs == std::vector<std::string>{ "qm1_1" });
    CHECK(arkAngels->ExitNpcs == std::vector<std::string>{ "qm2" });

    const auto* divineMight = find(*laloff, xi::ZoneId::LaloffAmphitheater, xi::BattlefieldId::DivineMight);
    REQUIRE(divineMight != nullptr);
    CHECK(divineMight->EntryNpcs.size() == 5);
    CHECK(divineMight->ExitNpcs.empty());

    const auto  modules      = std::array{ std::string{ "exit_npcs: [Burning_Circle_2]\nbattlefields:\n  tails_of_woe:\n    entry_npcs: [Other_Entrance]\n" } };
    const auto  battlefields = BattlefieldsDataset::decode(xi::data::mergeYaml(horlaisPeakFile(), modules));
    const auto* tailsOfWoe   = find(battlefields, xi::ZoneId::HorlaisPeak, xi::BattlefieldId::TailsOfWoe);
    REQUIRE(tailsOfWoe != nullptr);
    CHECK(tailsOfWoe->EntryNpcs == std::vector<std::string>{ "Other_Entrance" });
    CHECK(tailsOfWoe->ExitNpcs == std::vector<std::string>{ "Burning_Circle_2" });

    const auto* dismemberment = find(battlefields, xi::ZoneId::HorlaisPeak, xi::BattlefieldId::DismembermentBrigade);
    REQUIRE(dismemberment != nullptr);
    CHECK(dismemberment->EntryNpcs == std::vector<std::string>{ "BC_Entrance" });
}

TEST_CASE("battlefields: arenas must exist in the zone", "[data][battlefield]")
{
    const auto allowed = std::array{ std::string{ "battlefields:\n  tails_of_woe:\n    allowed_areas: [4]\n" } };
    CHECK_THROWS(BattlefieldsDataset::decode(xi::data::mergeYaml(horlaisPeakFile(), allowed)));

    const auto fixed = std::array{ std::string{ "battlefields:\n  tails_of_woe:\n    area: 4\n" } };
    CHECK_THROWS(BattlefieldsDataset::decode(xi::data::mergeYaml(horlaisPeakFile(), fixed)));

    const auto moreArenas = std::array{ std::string{ "max_areas: 4\nbattlefields:\n  tails_of_woe:\n    area: 4\n" } };
    CHECK_NOTHROW(BattlefieldsDataset::decode(xi::data::mergeYaml(horlaisPeakFile(), moreArenas)));
}

TEST_CASE("battlefields: a zone without a file hosts none", "[data][battlefield]")
{
    CHECK_FALSE(xi::data::loadZoneFile<BattlefieldsDataset>(xi::ZoneId::WestRonfaure).has_value());
}

TEST_CASE("battlefields: a module adds and removes battlefields in a zone", "[data][battlefield]")
{
    const auto modules      = std::array{ std::string{ "battlefields:\n  tails_of_woe: null\n  custom_trial: {index: 30}\n" } };
    const auto battlefields = BattlefieldsDataset::decode(xi::data::mergeYaml(horlaisPeakFile(), modules));

    CHECK(find(battlefields, xi::ZoneId::HorlaisPeak, xi::BattlefieldId::TailsOfWoe) == nullptr);

    const auto* custom = find(battlefields, xi::ZoneId::HorlaisPeak, xi::data::battlefieldId(xi::ZoneId::HorlaisPeak, 30));
    REQUIRE(custom != nullptr);
    CHECK(custom->Name == "custom_trial");
}

TEST_CASE("battlefields: an index is used once in a zone", "[data][battlefield]")
{
    const auto modules = std::array{ std::string{ "battlefields:\n  copied_trial: {index: 1}\n" } };

    CHECK_THROWS(BattlefieldsDataset::decode(xi::data::mergeYaml(horlaisPeakFile(), modules)));
}

TEST_CASE("battlefields: an index fits the entry menu", "[data][battlefield]")
{
    const auto modules = std::array{ std::string{ "battlefields:\n  overflow_trial: {index: 32}\n" } };

    CHECK_THROWS(BattlefieldsDataset::decode(xi::data::mergeYaml(horlaisPeakFile(), modules)));
}

TEST_CASE("battlefields: a known name stays in its zone and slot", "[data][battlefield]")
{
    const auto modules = std::array{ std::string{ "battlefields:\n  tails_of_woe: {index: 31}\n" } };
    const auto moved   = BattlefieldsDataset::decode(xi::data::mergeYaml(horlaisPeakFile(), modules));

    CHECK_THROWS(BattlefieldsDataset::verifyZone(moved, xi::ZoneId::HorlaisPeak));
    CHECK_THROWS(BattlefieldsDataset::verifyZone(BattlefieldsDataset::decode(horlaisPeakFile()), xi::ZoneId::MonarchLinn));
    CHECK_NOTHROW(BattlefieldsDataset::verifyZone(BattlefieldsDataset::decode(horlaisPeakFile()), xi::ZoneId::HorlaisPeak));
}
