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

#include "data/datasets/gardening_results/dataset.h"

#include "common/enum_traits.h"
#include "data/datasets/gardening_results/yaml.h"
#include "data/yaml/enum_keyed_map.h"
#include "data/yaml/read.h"

#include <fmt/format.h>

#include <algorithm>
#include <cstddef>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace xi::data::datasets::gardening_results
{

namespace
{

constexpr std::size_t kCrystals = EnumTraits<xi::Element>::kEntries.size();

auto label(const GardeningKey& key) -> std::string
{
    const auto& [plant, first, second] = key;
    return fmt::format("{} {} {}", EnumTraits<xi::FlowerpotPlant>::toName(plant), EnumTraits<xi::Element>::toName(first), EnumTraits<xi::Element>::toName(second));
}

void add(GardeningResults& records, const GardeningKey& key, const wire::Results& source)
{
    if (source.empty())
    {
        throw std::runtime_error(fmt::format("{} has no results", label(key)));
    }

    std::vector<GardeningResult> results;
    results.reserve(source.size());
    for (const auto& result : source)
    {
        const auto [minQuantity, maxQuantity] = result.quantity;
        if (minQuantity > maxQuantity)
        {
            throw std::runtime_error(fmt::format("{} item {} quantity [{}, {}] is reversed", label(key), result.item, minQuantity, maxQuantity));
        }

        results.push_back({ .ItemId = result.item, .MinQuantity = minQuantity, .MaxQuantity = maxQuantity, .Weight = result.weight });
    }

    if (!records.emplace(key, std::move(results)).second)
    {
        throw std::runtime_error(fmt::format("{} is listed twice", label(key)));
    }
}

auto planted(const GardeningResults& records, const xi::FlowerpotPlant plant) -> std::size_t
{
    return std::ranges::count_if(
        records | std::views::keys,
        [plant](const GardeningKey& key)
        {
            return std::get<0>(key) == plant;
        });
}

} // namespace

auto Dataset::decode(const std::string_view text) -> Records
{
    const auto source = yaml::read<YamlDocument>(text).gardening_results;
    const auto plants = yaml::resolveKeys<xi::FlowerpotPlant, wire::ByCrystal<wire::Results>>(source.plants);
    const auto trees  = yaml::resolveKeys<xi::FlowerpotPlant, wire::ByCrystal<wire::ByCrystal<wire::Results>>>(source.trees);

    GardeningResults records;

    for (const auto& [plant, byCrystal] : plants)
    {
        for (const auto& [crystal, results] : yaml::resolveKeys<xi::Element, wire::Results>(byCrystal))
        {
            add(records, { plant, xi::Element::None, crystal }, results);
        }
    }

    for (const auto& [plant, byFirst] : trees)
    {
        for (const auto& [first, bySecond] : yaml::resolveKeys<xi::Element, wire::ByCrystal<wire::Results>>(byFirst))
        {
            for (const auto& [second, results] : yaml::resolveKeys<xi::Element, wire::Results>(bySecond))
            {
                add(records, { plant, first, second }, results);
            }
        }

        for (const auto& first : EnumTraits<xi::Element>::kEntries)
        {
            for (const auto& second : EnumTraits<xi::Element>::kEntries)
            {
                if (const auto reverse = records.find({ plant, second.second, first.second }); reverse != records.end())
                {
                    records.try_emplace({ plant, first.second, second.second }, reverse->second);
                }
            }
        }
    }

    for (const auto& [name, plant] : EnumTraits<xi::FlowerpotPlant>::kEntries)
    {
        const auto isPlant = plants.contains(plant);
        const auto isTree  = trees.contains(plant);
        if (plant == xi::FlowerpotPlant::None)
        {
            if (isPlant || isTree)
            {
                throw std::runtime_error("none cannot hold results");
            }

            continue;
        }

        if (isPlant == isTree)
        {
            throw std::runtime_error(fmt::format("{} has to be listed once, under plants or trees", name));
        }

        if (isPlant && planted(records, plant) != kCrystals)
        {
            throw std::runtime_error(fmt::format("{} has to list every crystal", name));
        }

        if (isTree && planted(records, plant) != kCrystals * kCrystals)
        {
            throw std::runtime_error(fmt::format("{} has to list every pair of crystals", name));
        }
    }

    return records;
}

} // namespace xi::data::datasets::gardening_results
