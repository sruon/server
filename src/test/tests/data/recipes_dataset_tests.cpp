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
#include "data/enums/craft_rank.h"
#include "data/enums/element.h"
#include "data/enums/key_item.h"
#include "map/data/datasets/synergy_recipes/dataset.h"
#include "map/data/datasets/synth_recipes/dataset.h"
#include "map/data/yaml/merge.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

namespace
{

using SynthRecipesDataset   = xi::data::datasets::synth_recipes::Dataset;
using SynergyRecipesDataset = xi::data::datasets::synergy_recipes::Dataset;

auto slurp(const std::string& path) -> std::string
{
    std::ifstream input(path, std::ios::binary);
    return { std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };
}

auto findRecipe(const xi::data::SynthRecipes& recipes, const uint32 id) -> const xi::data::SynthRecipeData*
{
    const auto it = std::ranges::find(recipes, id, &xi::data::SynthRecipeData::Id);
    if (it == recipes.end())
    {
        return nullptr;
    }

    return &*it;
}

auto resultItems(const xi::data::RecipeItemsData& items) -> std::vector<std::string>
{
    std::vector<std::string> names;
    for (const auto& result : items.Results)
    {
        names.push_back(result.Item);
    }

    return names;
}

auto resultQuantities(const xi::data::RecipeItemsData& items) -> std::vector<uint8>
{
    std::vector<uint8> quantities;
    for (const auto& result : items.Results)
    {
        quantities.push_back(result.Quantity);
    }

    return quantities;
}

} // namespace

TEST_CASE("synth_recipes: recipes read from data/synth_recipes.yaml", "[data][recipes]")
{
    const auto recipes = SynthRecipesDataset::decode(slurp("data/synth_recipes.yaml"));
    CHECK(std::ranges::is_sorted(recipes, {}, &xi::data::SynthRecipeData::Id));

    const auto* lumber = findRecipe(recipes, 3);
    REQUIRE(lumber != nullptr);
    CHECK(lumber->Name == "Arrowwood Lumber");
    CHECK_FALSE(lumber->Desynth);
    CHECK(lumber->Skills == std::array<uint8, 8>{ 2, 0, 0, 0, 0, 0, 0, 0 });
    CHECK(lumber->KeyItem == xi::KeyItem::Lumberjack);
    CHECK(lumber->Content == xi::Content::Cop);
    CHECK(lumber->Crystal == xi::Element::Wind);
    CHECK(lumber->Items.Ingredients == std::vector<std::pair<std::string, uint8>>{ { "arrowwood_log", 3 }, { "spool_of_bundling_twine", 1 } });
    CHECK(resultQuantities(lumber->Items) == std::vector<uint8>{ 3, 6, 9, 12 });
    CHECK(resultItems(lumber->Items) == std::vector<std::string>(4, "piece_of_arrowwood_lumber"));

    const auto* flute = findRecipe(recipes, 4);
    REQUIRE(flute != nullptr);
    CHECK(resultItems(flute->Items) == std::vector<std::string>{ "flute", "flute_+1", "flute_+2", "flute_+2" });
    CHECK(resultQuantities(flute->Items) == std::vector<uint8>{ 1, 1, 1, 1 });

    const auto* shield = findRecipe(recipes, 14);
    REQUIRE(shield != nullptr);
    CHECK(shield->Skills == std::array<uint8, 8>{ 7, 2, 0, 0, 0, 0, 0, 0 });

    const auto* desynth = findRecipe(recipes, 16);
    REQUIRE(desynth != nullptr);
    CHECK(desynth->Desynth);
    CHECK(desynth->Crystal == xi::Element::Thunder);

    CHECK(findRecipe(recipes, 505) == nullptr);
}

TEST_CASE("synth_recipes: a module enables a recipe and changes its ingredients", "[data][recipes]")
{
    const auto modules = std::array{ std::string{ "synth_recipes:\n"
                                                  "  cooking:\n"
                                                  "    505:\n"
                                                  "      enabled: true\n"
                                                  "      level: 11\n"
                                                  "      ingredients: {bamboo_stick: null, '5789': 1}\n" } };

    const auto recipes = SynthRecipesDataset::decode(xi::data::mergeYaml(slurp("data/synth_recipes.yaml"), modules));

    const auto* mandarin = findRecipe(recipes, 505);
    REQUIRE(mandarin != nullptr);
    CHECK(mandarin->Skills == std::array<uint8, 8>{ 11, 0, 0, 0, 0, 0, 0, 11 });
    CHECK(mandarin->Items.Ingredients == std::vector<std::pair<std::string, uint8>>{ { "5789", 1 }, { "saruta_orange", 4 } });
}

TEST_CASE("synth_recipes: subcrafts stay at or below the listed craft", "[data][recipes]")
{
    const auto modules = std::array{ std::string{ "synth_recipes:\n  woodworking:\n    14:\n      subcrafts: {smithing: 8}\n" } };

    CHECK_THROWS(SynthRecipesDataset::decode(xi::data::mergeYaml(slurp("data/synth_recipes.yaml"), modules)));
}

TEST_CASE("synth_recipes: a recipe has at most 8 ingredients", "[data][recipes]")
{
    const auto modules = std::array{ std::string{ "synth_recipes:\n  woodworking:\n    2:\n      ingredients: {arrowwood_log: 9}\n" } };

    CHECK_THROWS(SynthRecipesDataset::decode(xi::data::mergeYaml(slurp("data/synth_recipes.yaml"), modules)));
}

TEST_CASE("synergy_recipes: recipes read from data/synergy_recipes.yaml", "[data][recipes]")
{
    const auto recipes = SynergyRecipesDataset::decode(slurp("data/synergy_recipes.yaml"));
    REQUIRE(recipes.size() == 7);

    const auto& excalipoor = recipes[1];
    CHECK(excalipoor.Id == 2);
    CHECK(excalipoor.Name == "Excalipoor II");
    CHECK(excalipoor.Rank == xi::CraftRank::Amateur);
    CHECK(excalipoor.Support.empty());
    CHECK(excalipoor.Fewell == std::array<uint8, 8>{ 0, 0, 0, 0, 0, 0, 5, 5 });
    CHECK(excalipoor.Items.Ingredients == std::vector<std::pair<std::string, uint8>>{ { "excalipoor", 2 } });
    CHECK(resultItems(excalipoor.Items) == std::vector<std::string>(4, "excalipoor_ii"));
}
