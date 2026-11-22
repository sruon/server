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

#include "data/enums/mob_spell_list.h"
#include "map/data/datasets/mob_spell_lists/dataset.h"
#include "map/data/yaml/merge.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>

namespace
{

using MobSpellListsDataset = xi::data::datasets::mob_spell_lists::Dataset;

auto slurp(const std::string& path) -> std::string
{
    std::ifstream input(path, std::ios::binary);
    return { std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };
}

auto findList(const xi::data::MobSpellLists& lists, const xi::MobSpellList id) -> const xi::data::MobSpellListData&
{
    const auto it = std::ranges::find(lists, std::to_underlying(id), &xi::data::MobSpellListData::Id);
    REQUIRE(it != lists.end());
    return *it;
}

auto findSpell(const xi::data::MobSpellListData& list, const std::string& name) -> const xi::data::MobSpellData&
{
    const auto it = std::ranges::find(list.Spells, name, &xi::data::MobSpellData::Name);
    REQUIRE(it != list.Spells.end());
    return *it;
}

} // namespace

TEST_CASE("mob_spell_lists: levels read from data/mob_spell_lists.yaml", "[data][mobspelllist]")
{
    const auto lists = MobSpellListsDataset::decode(slurp("data/mob_spell_lists.yaml"));

    CHECK(std::ranges::is_sorted(lists, {}, &xi::data::MobSpellListData::Id));

    const auto& beastmenRdm = findList(lists, xi::MobSpellList::BeastmenRdm);
    CHECK(beastmenRdm.Id == 3);
    CHECK(beastmenRdm.Name == "beastmen_rdm");

    const auto& cure = findSpell(beastmenRdm, "cure");
    CHECK(cure.MinLevel == 3);
    CHECK(cure.MaxLevel == 13);

    const auto& dia = findSpell(beastmenRdm, "dia");
    CHECK(dia.MinLevel == 1);
    CHECK(dia.MaxLevel == 30);

    const auto& defaultBuff = findList(lists, xi::MobSpellList::DefaultBuff);
    REQUIRE(defaultBuff.Spells.size() == 1);
    CHECK(defaultBuff.Spells.front().MinLevel == 0);
    CHECK(defaultBuff.Spells.front().MaxLevel == 255);
}

TEST_CASE("mob_spell_lists: a module patch changes, adds and removes spells", "[data][mobspelllist]")
{
    const auto modules = std::array{ std::string{ R"(
mob_spell_lists:
  beastmen_rdm:
    spells:
      cure:     {max_level: 20}
      cure_ii:  null
      firaga:   {min_level: 50}
  custom_list:
    id: 9000
    spells:
      stone: {}
)" } };

    const auto lists = MobSpellListsDataset::decode(xi::data::mergeYaml(slurp("data/mob_spell_lists.yaml"), modules));

    const auto& beastmenRdm = findList(lists, xi::MobSpellList::BeastmenRdm);
    CHECK(findSpell(beastmenRdm, "cure").MinLevel == 3);
    CHECK(findSpell(beastmenRdm, "cure").MaxLevel == 20);
    CHECK(findSpell(beastmenRdm, "firaga").MinLevel == 50);
    CHECK(std::ranges::find(beastmenRdm.Spells, std::string{ "cure_ii" }, &xi::data::MobSpellData::Name) == beastmenRdm.Spells.end());

    const auto custom = std::ranges::find(lists, uint16{ 9000 }, &xi::data::MobSpellListData::Id);
    REQUIRE(custom != lists.end());
    CHECK(custom->Name == "custom_list");
    CHECK(lists.back().Id == 9000);
}

TEST_CASE("mob_spell_lists: rejects reused ids, id 0 and inverted levels", "[data][mobspelllist]")
{
    const auto core = slurp("data/mob_spell_lists.yaml");

    const auto reusedId = std::array{ std::string{ "mob_spell_lists:\n  custom_list:\n    id: 3\n    spells:\n      stone: {}\n" } };
    CHECK_THROWS(MobSpellListsDataset::decode(xi::data::mergeYaml(core, reusedId)));

    const auto reservedId = std::array{ std::string{ "mob_spell_lists:\n  custom_list:\n    id: 0\n    spells:\n      stone: {}\n" } };
    CHECK_THROWS(MobSpellListsDataset::decode(xi::data::mergeYaml(core, reservedId)));

    const auto inverted = std::array{ std::string{ "mob_spell_lists:\n  beastmen_rdm:\n    spells:\n      cure: {min_level: 20, max_level: 10}\n" } };
    CHECK_THROWS(MobSpellListsDataset::decode(xi::data::mergeYaml(core, inverted)));
}
