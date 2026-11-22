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

#include "data/datasets/synergy_recipes/dataset.h"

#include "data/datasets/synergy_recipes/yaml.h"
#include "data/yaml/read.h"

#include <fmt/format.h>

#include <stdexcept>
#include <utility>

namespace xi::data::datasets::synergy_recipes
{

auto Dataset::decode(const std::string_view text) -> Records
{
    const auto document = yaml::read<YamlDocument>(text);

    Records records;
    for (const auto& [id, source] : document.synergy_recipes)
    {
        const auto context = fmt::format("synergy recipe {}", id);

        SynergyRecipeData record{
            .Id    = id,
            .Name  = source.name,
            .Rank  = yaml::resolveEnum(source.rank),
            .Items = shared::toRecipeItems(source.ingredients, source.results, context),
        };

        for (const auto& support : source.support.value_or(std::vector<wire::Support>{}))
        {
            const auto skill = yaml::resolveEnum(support.skill);
            if (skill < xi::SkillType::Fishing || skill > xi::SkillType::Cooking)
            {
                throw std::runtime_error(fmt::format("{} has a support skill that is not a craft", context));
            }

            record.Support.push_back({ .Skill = skill, .Rank = yaml::resolveEnum(support.rank) });
        }

        if (record.Support.size() > 2)
        {
            throw std::runtime_error(fmt::format("{} has more than two support crafts", context));
        }

        for (const auto& [element, cost] : yaml::resolveKeys(source.fewell))
        {
            if (element == xi::Element::None)
            {
                throw std::runtime_error(fmt::format("{} burns fewell of no element", context));
            }

            record.Fewell[static_cast<size_t>(element) - 1] = cost;
        }

        records.emplace_back(std::move(record));
    }

    return records;
}

} // namespace xi::data::datasets::synergy_recipes
