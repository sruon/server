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

#include "data/enums/craft_rank.h"
#include "data/enums/guild.h"
#include "map/data/datasets/guilds/dataset.h"
#include "map/data/yaml/merge.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <fstream>
#include <iterator>
#include <string>

namespace
{

using GuildsDataset = xi::data::datasets::guilds::Dataset;

auto slurp(const std::string& path) -> std::string
{
    std::ifstream input(path, std::ios::binary);
    return { std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };
}

auto pattern(const xi::data::Guilds& guilds, const xi::Guild guild, const xi::CraftRank rank, const std::size_t index) -> const xi::data::GuildPointPattern&
{
    const auto it = std::ranges::find(guilds, guild, &xi::data::GuildData::Id);
    REQUIRE(it != guilds.end());
    const auto ranks = it->Ranks.find(rank);
    REQUIRE(ranks != it->Ranks.end());
    return ranks->second.at(index);
}

auto points(const xi::data::GuildPointPattern& pattern, const std::string& item) -> uint16
{
    const auto it = std::ranges::find(pattern.Items, item, &xi::data::GuildPointItem::Item);
    if (it == pattern.Items.end())
    {
        return 0;
    }

    return it->Points;
}

} // namespace

TEST_CASE("guilds: currencies and point items read from data/guilds.yaml", "[data][guild]")
{
    const auto guilds = GuildsDataset::decode(slurp("data/guilds.yaml"));

    REQUIRE(guilds.size() == 9);
    for (std::size_t index = 0; index < guilds.size(); ++index)
    {
        CHECK(static_cast<std::size_t>(guilds[index].Id) == index);
        CHECK(guilds[index].Ranks.size() == 10);
    }

    CHECK(guilds[static_cast<std::size_t>(xi::Guild::Clothcraft)].Currency == "guild_weaving");

    const auto& amateurA = pattern(guilds, xi::Guild::Fishing, xi::CraftRank::Amateur, 0);
    CHECK(amateurA.MaxPoints == 3600);
    CHECK(points(amateurA, "bastore_sardine") == 24);

    const auto& meuniere = pattern(guilds, xi::Guild::Cooking, xi::CraftRank::Adept, 4);
    CHECK(meuniere.MaxPoints == 12240);
    CHECK(points(meuniere, "serving_of_bass_meuniere") == 457);
    CHECK(points(meuniere, "serving_of_bass_meuniere_+1") == 915);
}

TEST_CASE("guilds: abyssea module restores the pre-2011 items", "[data][guild]")
{
    const auto modules = std::array{ slurp("modules/era/data/abyssea/guilds.yaml") };
    const auto guilds  = GuildsDataset::decode(xi::data::mergeYaml(slurp("data/guilds.yaml"), modules));

    const auto& amateurD = pattern(guilds, xi::Guild::Fishing, xi::CraftRank::Amateur, 3);
    CHECK(amateurD.MaxPoints == 1120);
    CHECK(points(amateurD, "phanauet_newt") == 12);
    CHECK(points(amateurD, "moat_carp") == 0);

    const auto& woodworkingD = pattern(guilds, xi::Guild::Woodworking, xi::CraftRank::Amateur, 3);
    CHECK(points(woodworkingD, "ash_staff") == 16);
    CHECK(points(woodworkingD, "ash_staff_+1") == 24);
    CHECK(points(woodworkingD, "workbench") == 0);
}

TEST_CASE("guilds: rov module divides the daily maximums by three", "[data][guild]")
{
    const auto modules = std::array{ slurp("modules/era/data/rov/guilds.yaml") };
    const auto guilds  = GuildsDataset::decode(xi::data::mergeYaml(slurp("data/guilds.yaml"), modules));

    const auto& amateurA = pattern(guilds, xi::Guild::Fishing, xi::CraftRank::Amateur, 0);
    CHECK(amateurA.MaxPoints == 1200);
    CHECK(points(amateurA, "bastore_sardine") == 24);
}

TEST_CASE("guilds: a module patch swaps one pattern's item", "[data][guild]")
{
    const auto modules = std::array{ std::string{ "guilds:\n  fishing:\n    ranks:\n      amateur:\n        a: {max_points: 1000, items: {bastore_sardine: null, crayfish: 50}}\n" } };
    const auto guilds  = GuildsDataset::decode(xi::data::mergeYaml(slurp("data/guilds.yaml"), modules));

    const auto& amateurA = pattern(guilds, xi::Guild::Fishing, xi::CraftRank::Amateur, 0);
    CHECK(amateurA.MaxPoints == 1000);
    CHECK(amateurA.Items.size() == 1);
    CHECK(points(amateurA, "crayfish") == 50);
    CHECK(pattern(guilds, xi::Guild::Fishing, xi::CraftRank::Amateur, 1).MaxPoints == 3840);
}

TEST_CASE("guilds: a pattern must request an item worth points", "[data][guild]")
{
    const auto empty = std::array{ std::string{ "guilds:\n  fishing:\n    ranks:\n      amateur:\n        a: {items: {bastore_sardine: null}}\n" } };
    CHECK_THROWS(GuildsDataset::decode(xi::data::mergeYaml(slurp("data/guilds.yaml"), empty)));

    const auto worthless = std::array{ std::string{ "guilds:\n  fishing:\n    ranks:\n      amateur:\n        a: {items: {bastore_sardine: 0}}\n" } };
    CHECK_THROWS(GuildsDataset::decode(xi::data::mergeYaml(slurp("data/guilds.yaml"), worthless)));
}

TEST_CASE("guilds: every guild must be listed", "[data][guild]")
{
    const auto modules = std::array{ std::string{ "guilds:\n  cooking: null\n" } };
    CHECK_THROWS(GuildsDataset::decode(xi::data::mergeYaml(slurp("data/guilds.yaml"), modules)));
}
