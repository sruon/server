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

#include "data/enums/automaton_frame.h"
#include "data/enums/automaton_head.h"
#include "data/enums/immunity.h"
#include "data/enums/status_effect.h"
#include "map/data/datasets/automaton_abilities/dataset.h"
#include "map/data/datasets/automaton_spells/dataset.h"
#include "map/data/yaml/merge.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

namespace
{

using AutomatonAbilitiesDataset = xi::data::datasets::automaton_abilities::Dataset;
using AutomatonSpellsDataset    = xi::data::datasets::automaton_spells::Dataset;

auto slurp(const std::string& path) -> std::string
{
    std::ifstream input(path, std::ios::binary);
    return { std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };
}

auto findSpell(const xi::data::AutomatonSpells& spells, const std::string_view name) -> const xi::data::AutomatonSpellData&
{
    const auto it = std::ranges::find(spells, name, &xi::data::AutomatonSpellData::Name);
    REQUIRE(it != spells.end());
    return *it;
}

} // namespace

TEST_CASE("automaton_abilities: frames and skill levels read from data/automaton_abilities.yaml", "[data][automaton]")
{
    const auto abilities = AutomatonAbilitiesDataset::decode(slurp("data/automaton_abilities.yaml"));

    REQUIRE(std::ranges::is_sorted(abilities, {}, &xi::data::AutomatonAbilityData::Id));

    const auto cannibalBlade = std::ranges::find(abilities, "cannibal_blade", &xi::data::AutomatonAbilityData::Name);
    REQUIRE(cannibalBlade != abilities.end());
    CHECK(cannibalBlade->Id == 2065);
    CHECK(cannibalBlade->Frame == xi::AutomatonFrame::Valoredge);
    CHECK(cannibalBlade->SkillLevel == 150);

    const auto provoke = std::ranges::find(abilities, "provoke", &xi::data::AutomatonAbilityData::Name);
    REQUIRE(provoke != abilities.end());
    CHECK(provoke->Frame == xi::AutomatonFrame::None);
    CHECK(provoke->SkillLevel == 0);
}

TEST_CASE("automaton_spells: heads, enfeebles and removals read from data/automaton_spells.yaml", "[data][automaton]")
{
    const auto spells = AutomatonSpellsDataset::decode(slurp("data/automaton_spells.yaml"));

    REQUIRE(std::ranges::is_sorted(spells, {}, &xi::data::AutomatonSpellData::Id));

    const auto& cure = findSpell(spells, "cure");
    CHECK(cure.Id == 1);
    CHECK(cure.SkillLevel == 12);
    CHECK(cure.Heads == std::vector{ xi::AutomatonHead::Harlequin, xi::AutomatonHead::Valoredge, xi::AutomatonHead::Sharpshot, xi::AutomatonHead::Stormwaker, xi::AutomatonHead::Soulsoother });
    CHECK(cure.Removes == std::vector{ xi::StatusEffect::Lullaby, xi::StatusEffect::SleepIi, xi::StatusEffect::SleepI });

    const auto& slow = findSpell(spells, "slow");
    CHECK(slow.Enfeeble == xi::StatusEffect::Slow);
    CHECK(slow.Immunity == xi::Immunity::Slow);
    CHECK(slow.Removes.empty());

    const auto& dia = findSpell(spells, "dia");
    CHECK(dia.SkillLevel == 0);
    CHECK(dia.Immunity == xi::Immunity::None);
}

TEST_CASE("automaton_spells: wotg era module raises skill levels and narrows heads", "[data][automaton]")
{
    const auto modules = std::array{ slurp("modules/era/data/wotg/automaton_spells.yaml") };
    const auto spells  = AutomatonSpellsDataset::decode(xi::data::mergeYaml(slurp("data/automaton_spells.yaml"), modules));

    CHECK(findSpell(spells, "cure_vi").SkillLevel == 999);
    CHECK(findSpell(spells, "cure_vi").Heads == std::vector{ xi::AutomatonHead::Soulsoother });
    CHECK(findSpell(spells, "fire_iv").Heads == std::vector{ xi::AutomatonHead::Spiritreaver });
    CHECK(findSpell(spells, "fire_iv").SkillLevel == 281);
    CHECK(findSpell(spells, "cure").SkillLevel == 12);
}

TEST_CASE("automaton_spells: a spell needs at least one head", "[data][automaton]")
{
    const auto modules = std::array{ std::string{ "automaton_spells:\n  cure:\n    heads: []\n" } };

    CHECK_THROWS(AutomatonSpellsDataset::decode(xi::data::mergeYaml(slurp("data/automaton_spells.yaml"), modules)));
}
