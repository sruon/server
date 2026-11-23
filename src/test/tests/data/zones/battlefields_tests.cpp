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
#include <string>

namespace
{

using BattlefieldsDataset = xi::data::datasets::zones::battlefields::Dataset;

auto horlaisPeakFile() -> std::string
{
    return xi::data::loadMergedYaml(xi::data::zoneFilePath(xi::ZoneId::HorlaisPeak, BattlefieldsDataset::kDataPath), {});
}

auto find(const xi::data::Battlefields& battlefields, const xi::ZoneId zone, const xi::BattlefieldId id) -> const xi::data::BattlefieldData*
{
    const auto it = std::ranges::find_if(battlefields,
                                         [&](const xi::data::BattlefieldData& entry)
                                         {
                                             return xi::data::battlefieldId(zone, entry.Index) == id;
                                         });
    if (it == battlefields.end())
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
    CHECK(std::ranges::is_sorted(*battlefields, {}, &xi::data::BattlefieldData::Index));

    const auto* tailsOfWoe = find(*battlefields, xi::ZoneId::HorlaisPeak, xi::BattlefieldId::TailsOfWoe);
    REQUIRE(tailsOfWoe != nullptr);
    CHECK(tailsOfWoe->Name == "tails_of_woe");
    CHECK(tailsOfWoe->Index == 1);
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
