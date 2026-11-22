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

#include "data/datasets/blue_traits/dataset.h"

#include "data/datasets/blue_traits/yaml.h"
#include "data/yaml/read.h"

#include <fmt/format.h>

#include <algorithm>
#include <stdexcept>
#include <unordered_set>

namespace xi::data::datasets::blue_traits
{

auto Dataset::decode(const std::string_view text) -> Records
{
    const auto document = yaml::read<YamlDocument>(text);

    Records                   records;
    std::unordered_set<uint8> ids;

    for (const auto& [name, source] : document.blue_traits)
    {
        if (source.id == 0 || !ids.insert(source.id).second)
        {
            throw std::runtime_error(fmt::format("blue trait {} has a zero or duplicate id {}", name, source.id));
        }

        if (source.tiers.empty())
        {
            throw std::runtime_error(fmt::format("blue trait {} has no tiers", name));
        }

        BlueTraitCategoryData category{ .Id = source.id, .Name = name };
        for (const auto& tier : source.tiers)
        {
            BlueTraitTierData record{
                .Rank          = tier.rank,
                .Points        = tier.points,
                .Trait         = tier.trait.value_or(source.trait),
                .JobPointsOnly = tier.job_points_only.value_or(false),
            };

            for (const auto& [mod, value] : yaml::resolveKeys(std::optional{ tier.mods }))
            {
                record.Mods.emplace_back(mod, value);
            }

            if (record.Mods.empty())
            {
                throw std::runtime_error(fmt::format("blue trait {} rank {} has no mods", name, tier.rank));
            }

            std::ranges::sort(record.Mods);
            category.Tiers.emplace_back(std::move(record));
        }

        records.emplace_back(std::move(category));
    }

    std::ranges::sort(records, {}, &BlueTraitCategoryData::Id);
    return records;
}

} // namespace xi::data::datasets::blue_traits
