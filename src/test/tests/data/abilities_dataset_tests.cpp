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

#include "data/enums/ability_add_type.h"
#include "data/enums/ability_category.h"
#include "data/enums/action_target.h"
#include "data/enums/content.h"
#include "data/enums/job.h"
#include "data/enums/job_ability.h"
#include "data/enums/merit.h"
#include "map/data/datasets/abilities/dataset.h"
#include "map/data/yaml/merge.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <fstream>
#include <iterator>
#include <string>

namespace
{

using AbilitiesDataset = xi::data::datasets::abilities::Dataset;

auto slurp(const std::string& path) -> std::string
{
    std::ifstream input(path, std::ios::binary);
    return { std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };
}

auto find(const xi::data::Abilities& abilities, const xi::JobAbility id) -> const xi::data::AbilityData&
{
    const auto it = std::ranges::find(abilities, id, &xi::data::AbilityData::Id);
    REQUIRE(it != abilities.end());
    return *it;
}

} // namespace

TEST_CASE("abilities: fields read from data/abilities.yaml", "[data][ability]")
{
    const auto abilities = AbilitiesDataset::decode(slurp("data/abilities.yaml"));

    const auto& provoke = find(abilities, xi::JobAbility::Provoke);
    CHECK(provoke.Job == xi::Job::WAR);
    CHECK(provoke.Level == 5);
    CHECK(provoke.Target == xi::ActionTarget::Hostile);
    CHECK(provoke.Range == 16.0f);
    CHECK(provoke.Category == xi::AbilityCategory::AbilityFinish);
    CHECK(provoke.AnimationTime == 2000);
    CHECK(provoke.Recast == 30);
    CHECK(provoke.CE == 1);
    CHECK(provoke.VE == 1800);
    CHECK_FALSE(provoke.Aoe);
    CHECK(provoke.Radius == 0);

    const auto& benediction = find(abilities, xi::JobAbility::Benediction);
    CHECK(benediction.Target == xi::ActionTarget::Self);
    CHECK(benediction.Aoe);
    CHECK(benediction.Radius == 14);

    const auto& dustCloud = find(abilities, xi::JobAbility::DustCloud);
    CHECK_FALSE(dustCloud.Aoe);
    CHECK(dustCloud.Radius == 10);

    CHECK(find(abilities, xi::JobAbility::CuttingCards).Target == (xi::ActionTarget::Self | xi::ActionTarget::Party));
    CHECK(find(abilities, xi::JobAbility::FireManeuver).AnimationTime == 0);
    CHECK(find(abilities, xi::JobAbility::HealingBreath).Job == xi::Job::NONE);
    CHECK(find(abilities, xi::JobAbility::HealingBreath).CastTime == 1500);
    CHECK(find(abilities, xi::JobAbility::Trance).Content == xi::Content::Wotg);
    CHECK(find(abilities, xi::JobAbility::Sic).AddType == xi::AbilityAddType::CharmPet);
    CHECK(find(abilities, xi::JobAbility::CuringWaltz).Category == xi::AbilityCategory::Dancer);

    // enum-only ids are left out of the records
    CHECK(std::ranges::find(abilities, xi::JobAbility::Enrage, &xi::data::AbilityData::Id) == abilities.end());
    CHECK(std::ranges::find(abilities, xi::JobAbility::WindsBlessing, &xi::data::AbilityData::Id) == abilities.end());
}

TEST_CASE("abilities: charges sit on the ability that opens them", "[data][ability]")
{
    const auto abilities = AbilitiesDataset::decode(slurp("data/abilities.yaml"));

    const auto& ready = find(abilities, xi::JobAbility::Ready);
    REQUIRE(ready.Charges.has_value());
    CHECK(ready.Charges->RecastId == 102);
    REQUIRE(ready.Charges->Tiers.size() == 1);
    CHECK(ready.Charges->Tiers[0].MaxCharges == 3);
    CHECK(ready.Charges->Tiers[0].ChargeTime == 30);
    CHECK(ready.Charges->Tiers[0].Merit == xi::Merit::SicRecast);

    const auto& stratagems = find(abilities, xi::JobAbility::Stratagems);
    REQUIRE(stratagems.Charges.has_value());
    CHECK(stratagems.Charges->RecastId == 231);
    CHECK(stratagems.Charges->Tiers.size() == 5);

    CHECK(find(abilities, xi::JobAbility::QuickDraw).Charges->RecastId == 195);
}

TEST_CASE("abilities: a module patch changes one ability by job and name", "[data][ability]")
{
    const auto modules   = std::array{ std::string{ "abilities:\n  war:\n    provoke: {recast: 15, enmity: {ce: 1, ve: 900}}\n" } };
    const auto abilities = AbilitiesDataset::decode(xi::data::mergeYaml(slurp("data/abilities.yaml"), modules));

    const auto& provoke = find(abilities, xi::JobAbility::Provoke);
    CHECK(provoke.Recast == 15);
    CHECK(provoke.VE == 900);
    CHECK(provoke.Range == 16.0f);
    CHECK(provoke.RecastId == 5);
}

TEST_CASE("abilities: era modules revert recasts, ranges and enmity", "[data][ability]")
{
    const auto modules = std::array{
        slurp("modules/era/data/wotg/abilities.yaml"),
        slurp("modules/era/data/abyssea/abilities.yaml"),
        slurp("modules/era/data/soa/abilities.yaml"),
        slurp("modules/era/data/rov/abilities.yaml"),
    };
    const auto abilities = AbilitiesDataset::decode(xi::data::mergeYaml(slurp("data/abilities.yaml"), modules));

    const auto& martyr = find(abilities, xi::JobAbility::Martyr);
    CHECK(martyr.Range == 6.0f);
    CHECK(martyr.Recast == 1200);

    const auto& sic = find(abilities, xi::JobAbility::Sic);
    CHECK(sic.Recast == 120);
    CHECK(sic.AddType == (xi::AbilityAddType::JugPet | xi::AbilityAddType::CharmPet));

    const auto& jump = find(abilities, xi::JobAbility::Jump);
    CHECK(jump.Recast == 90);
    CHECK(jump.RecastId == 158);

    const auto& rampart = find(abilities, xi::JobAbility::Rampart);
    CHECK(rampart.Recast == 300);
    CHECK(rampart.CE == 1);
    CHECK(rampart.VE == 300);

    CHECK(find(abilities, xi::JobAbility::MightyStrikes).Recast == 7200);
    CHECK(find(abilities, xi::JobAbility::FireManeuver).Animation == 83);
    CHECK(find(abilities, xi::JobAbility::FireManeuver).AnimationTime == 2000);
}

TEST_CASE("abilities: each era module only touches its own expansion", "[data][ability]")
{
    const auto modules   = std::array{ slurp("modules/era/data/rov/abilities.yaml") };
    const auto abilities = AbilitiesDataset::decode(xi::data::mergeYaml(slurp("data/abilities.yaml"), modules));

    CHECK(find(abilities, xi::JobAbility::Boost).Recast == 15);
    CHECK(find(abilities, xi::JobAbility::Martyr).Recast == 600);
    CHECK(find(abilities, xi::JobAbility::MightyStrikes).Recast == 3600);
}

TEST_CASE("abilities: a recast holds charges once", "[data][ability]")
{
    const auto modules = std::array{ std::string{ "abilities:\n  bst:\n    sic:\n      charges:\n        tiers:\n          - {level: 25, max: 3, time: 30}\n" } };

    CHECK_THROWS(AbilitiesDataset::decode(xi::data::mergeYaml(slurp("data/abilities.yaml"), modules)));
}

TEST_CASE("abilities: groups must be job names", "[data][ability]")
{
    CHECK_THROWS(AbilitiesDataset::decode("abilities:\n  warrior:\n    provoke: {id: 35, animation: 3}\n"));
}
