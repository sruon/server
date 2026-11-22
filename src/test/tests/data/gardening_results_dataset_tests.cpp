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

#include "map/data/datasets/gardening_results/dataset.h"
#include "map/data/yaml/merge.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace
{

using GardeningResultsDataset = xi::data::datasets::gardening_results::Dataset;

using xi::Element;
using xi::FlowerpotPlant;

auto slurp(const std::string& path) -> std::string
{
    std::ifstream input(path, std::ios::binary);
    return { std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };
}

auto items(const std::vector<xi::data::GardeningResult>& results) -> std::vector<uint16>
{
    std::vector<uint16> ids;
    std::ranges::transform(results, std::back_inserter(ids), &xi::data::GardeningResult::ItemId);
    return ids;
}

} // namespace

TEST_CASE("gardening results: read from data/gardening_results.yaml", "[data][gardening_results]")
{
    const auto results = GardeningResultsDataset::decode(slurp("data/gardening_results.yaml"));

    CHECK(results.size() == 360);

    const auto& herbs = results.at({ FlowerpotPlant::HerbSeeds, Element::None, Element::None });
    REQUIRE(herbs.size() == 4);
    CHECK(herbs[0].ItemId == 936);
    CHECK(herbs[0].MinQuantity == 12);
    CHECK(herbs[0].MaxQuantity == 24);
    CHECK(herbs[0].Weight == 30);

    CHECK(items(results.at({ FlowerpotPlant::FruitSeeds, Element::Fire, Element::Ice })) == std::vector<uint16>{ 4097, 4098, 4392, 4390, 4445, 4503, 769, 773 });
}

TEST_CASE("gardening results: era abyssea patch removes Burdock and Winterflower", "[data][gardening_results]")
{
    const std::vector<std::string> modules{ "modules/era/data/abyssea/gardening_results.yaml" };
    const auto                     core    = GardeningResultsDataset::decode(slurp("data/gardening_results.yaml"));
    const auto                     patched = GardeningResultsDataset::decode(xi::data::loadMergedYaml("data/gardening_results.yaml", modules));

    CHECK(items(core.at({ FlowerpotPlant::VegetableSeeds, Element::None, Element::Fire })) == std::vector<uint16>{ 4097, 619, 5651 });
    CHECK(items(patched.at({ FlowerpotPlant::VegetableSeeds, Element::None, Element::Fire })) == std::vector<uint16>{ 4097, 619 });
    CHECK(items(patched.at({ FlowerpotPlant::VegetableSeeds, Element::None, Element::Water })) == std::vector<uint16>{ 4096, 4366 });

    auto untouched = 0;
    for (const auto& [key, list] : core)
    {
        if (items(patched.at(key)) == items(list))
        {
            ++untouched;
        }
    }

    CHECK(untouched == 358);
}

TEST_CASE("gardening results: every crystal has to be listed", "[data][gardening_results]")
{
    auto       text = slurp("data/gardening_results.yaml");
    const auto at   = text.find("\n      fire:");
    text.erase(at, text.find("\n      ice:", at) - at);

    CHECK_THROWS(GardeningResultsDataset::decode(text));
}

TEST_CASE("gardening results: malformed results are rejected", "[data][gardening_results]")
{
    const auto core  = slurp("data/gardening_results.yaml");
    const auto patch = [&](const std::string& results)
    {
        const std::vector<std::string> modules{ R"(
gardening_results:
  plants:
    herb_seeds:
      none: )" + results };

        return GardeningResultsDataset::decode(xi::data::mergeYaml(core, modules));
    };

    CHECK(patch("[{item: 936, quantity: [12, 24], weight: 100}]").at({ FlowerpotPlant::HerbSeeds, Element::None, Element::None }).size() == 1);
    CHECK_THROWS(patch("[{item: 936, quantity: [24, 12], weight: 100}]"));
    CHECK_THROWS(patch("[]"));
    CHECK_THROWS(patch("[{item: 936, quantity: [12, 24], weight: 100, extra: 1}]"));
}

TEST_CASE("gardening results: a plant listed under plants and trees is rejected", "[data][gardening_results]")
{
    const std::vector<std::string> modules{ R"(
gardening_results:
  trees:
    herb_seeds:
      none:
        none: [{item: 936, quantity: [1, 1], weight: 100}]
)" };

    CHECK_THROWS(GardeningResultsDataset::decode(xi::data::mergeYaml(slurp("data/gardening_results.yaml"), modules)));
}

TEST_CASE("gardening results: a tree pair applies in both feed orders unless the reverse is listed", "[data][gardening_results]")
{
    const auto core = slurp("data/gardening_results.yaml");
    const auto base = GardeningResultsDataset::decode(core);

    CHECK(items(base.at({ FlowerpotPlant::FruitSeeds, Element::Ice, Element::Fire })) == items(base.at({ FlowerpotPlant::FruitSeeds, Element::Fire, Element::Ice })));

    const std::vector<std::string> modules{ R"(
gardening_results:
  trees:
    fruit_seeds:
      ice:
        fire: [{item: 936, quantity: [1, 1], weight: 100}]
)" };
    const auto                     patched = GardeningResultsDataset::decode(xi::data::mergeYaml(core, modules));

    CHECK(items(patched.at({ FlowerpotPlant::FruitSeeds, Element::Ice, Element::Fire })) == std::vector<uint16>{ 936 });
    CHECK(items(patched.at({ FlowerpotPlant::FruitSeeds, Element::Fire, Element::Ice })) == items(base.at({ FlowerpotPlant::FruitSeeds, Element::Fire, Element::Ice })));
}
