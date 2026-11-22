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

#include "data/enums/content.h"
#include "data/enums/job.h"
#include "data/enums/mod.h"
#include "data/enums/skillchain_type.h"
#include "data/enums/spell_aoe.h"
#include "data/enums/spell_group.h"
#include "data/enums/target_type.h"
#include "map/data/datasets/blue_traits/dataset.h"
#include "map/data/datasets/spells/dataset.h"
#include "map/data/yaml/merge.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

namespace
{

using SpellsDataset     = xi::data::datasets::spells::Dataset;
using BlueTraitsDataset = xi::data::datasets::blue_traits::Dataset;
using namespace std::chrono_literals;

auto slurp(const std::string& path) -> std::string
{
    std::ifstream input(path, std::ios::binary);
    return { std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };
}

auto find(const xi::data::Spells& spells, const std::string& name) -> const xi::data::SpellData&
{
    const auto it = std::ranges::find(spells, name, &xi::data::SpellData::Name);
    REQUIRE(it != spells.end());
    return *it;
}

} // namespace

TEST_CASE("spells: fields read from data/spells.yaml", "[data][spell]")
{
    const auto spells = SpellsDataset::decode(slurp("data/spells.yaml"));

    const auto& cureIi = find(spells, "cure_ii");
    CHECK(cureIi.Id == 2);
    CHECK(cureIi.Group == xi::SpellGroup::White);
    CHECK(cureIi.JobLevels[static_cast<uint8>(xi::Job::WHM)] == 11);
    CHECK(cureIi.JobLevels[static_cast<uint8>(xi::Job::BLM)] == 0);
    CHECK(cureIi.CastTime == 2250ms);
    CHECK(cureIi.RecastTime == 5500ms);
    CHECK(cureIi.AnimationTime == 2000ms);
    CHECK(cureIi.Range == 20.0f);
    CHECK(cureIi.Aoe == xi::SpellAoe::RadialAcce);
    CHECK(cureIi.Multiplier == 1.0f);
    CHECK(cureIi.Content == xi::Content::None);
    CHECK((cureIi.Targets & xi::TargetType::Self) == xi::TargetType::Self);
    CHECK_FALSE(cureIi.BlueMagic.has_value());

    const auto& shantotto = find(spells, "shantotto");
    CHECK(shantotto.Group == xi::SpellGroup::Trust);
    CHECK(shantotto.AnimationTime == 1500ms);
}

TEST_CASE("spells: blue magic is nested in its spell", "[data][spell]")
{
    const auto spells = SpellsDataset::decode(slurp("data/spells.yaml"));

    const auto& venomShell = find(spells, "venom_shell");
    CHECK(venomShell.Content == xi::Content::Toau);
    REQUIRE(venomShell.BlueMagic.has_value());
    CHECK(venomShell.BlueMagic->MonsterSkills == std::vector<uint16>{ 505 });
    CHECK(venomShell.BlueMagic->SetPoints == 3);
    CHECK(venomShell.BlueMagic->Trait == "clear_mind");
    CHECK(venomShell.BlueMagic->TraitWeight == 1);
    CHECK(venomShell.BlueMagic->Mods == std::vector<std::pair<xi::Mod, int16>>{ { xi::Mod::MND, 2 } });

    const auto& plenilune = find(spells, "plenilune_embrace");
    REQUIRE(plenilune.BlueMagic.has_value());
    CHECK(plenilune.BlueMagic->MonsterSkills == std::vector<uint16>{ 2173, 2174 });
    CHECK(plenilune.BlueMagic->Trait.empty());
    CHECK(plenilune.BlueMagic->TraitWeight == 0);
}

TEST_CASE("spells: every blue trait a spell names exists", "[data][spell]")
{
    const auto spells     = SpellsDataset::decode(slurp("data/spells.yaml"));
    const auto blueTraits = BlueTraitsDataset::decode(slurp("data/blue_traits.yaml"));

    for (const auto& spell : spells)
    {
        if (spell.BlueMagic && !spell.BlueMagic->Trait.empty())
        {
            INFO(spell.Name);
            CHECK(std::ranges::find(blueTraits, spell.BlueMagic->Trait, &xi::data::BlueTraitCategoryData::Name) != blueTraits.end());
        }
    }
}

TEST_CASE("blue_traits: tiers read from data/blue_traits.yaml", "[data][spell]")
{
    const auto blueTraits = BlueTraitsDataset::decode(slurp("data/blue_traits.yaml"));

    const auto clearMind = std::ranges::find(blueTraits, "clear_mind", &xi::data::BlueTraitCategoryData::Name);
    REQUIRE(clearMind != blueTraits.end());
    CHECK(clearMind->Id == 4);
    REQUIRE(clearMind->Tiers.size() == 6);
    CHECK(clearMind->Tiers[2].Rank == 3);
    CHECK(clearMind->Tiers[2].Points == 6);
    CHECK(clearMind->Tiers[2].Trait == 24);
    CHECK(clearMind->Tiers[2].Mods.size() == 2);
    CHECK_FALSE(clearMind->Tiers[2].JobPointsOnly);
    CHECK(clearMind->Tiers[4].JobPointsOnly);
}

TEST_CASE("spells: era modules patch by spell name", "[data][spell]")
{
    const auto modules = std::array{ slurp("modules/era/data/rov/spells.yaml"), slurp("modules/era/data/abyssea/spells.yaml") };
    const auto spells  = SpellsDataset::decode(xi::data::mergeYaml(slurp("data/spells.yaml"), modules));

    const auto& banishga = find(spells, "banishga");
    CHECK(banishga.JobLevels[static_cast<uint8>(xi::Job::WHM)] == 15);
    CHECK(banishga.JobLevels[static_cast<uint8>(xi::Job::PLD)] == 0);

    const auto& sinewyEtude = find(spells, "sinewy_etude");
    CHECK(sinewyEtude.CastTime == 4000ms);
    CHECK(sinewyEtude.Aoe == xi::SpellAoe::None);
    CHECK(sinewyEtude.CumulativeEnmity == 160);
    CHECK(sinewyEtude.VolatileEnmity == 160);
}

TEST_CASE("spells: a module patch can change one field", "[data][spell]")
{
    const auto modules = std::array{ std::string{ "spells:\n  cure:\n    mp_cost: 10\n    blue_magic: null\n  venom_shell:\n    blue_magic:\n      mods: {mnd: 5}\n" } };
    const auto spells  = SpellsDataset::decode(xi::data::mergeYaml(slurp("data/spells.yaml"), modules));

    CHECK(find(spells, "cure").MpCost == 10);
    CHECK(find(spells, "cure").CastTime == 2000ms);

    const auto& venomShell = find(spells, "venom_shell");
    REQUIRE(venomShell.BlueMagic.has_value());
    CHECK(venomShell.BlueMagic->SetPoints == 3);
    CHECK(venomShell.BlueMagic->Mods == std::vector<std::pair<xi::Mod, int16>>{ { xi::Mod::MND, 5 } });
}

TEST_CASE("spells: blue_magic outside the blue group is rejected", "[data][spell]")
{
    const auto modules = std::array{ std::string{ "spells:\n  cure:\n    blue_magic:\n      monster_skills: [1]\n      set_points: 1\n" } };

    CHECK_THROWS(SpellsDataset::decode(xi::data::mergeYaml(slurp("data/spells.yaml"), modules)));
}
