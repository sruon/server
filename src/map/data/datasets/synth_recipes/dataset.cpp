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

#include "data/datasets/synth_recipes/dataset.h"

#include "data/datasets/synth_recipes/yaml.h"
#include "data/yaml/read.h"

#include <fmt/format.h>

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace xi::data::datasets::synth_recipes
{

auto Dataset::decode(const std::string_view text) -> Records
{
    const auto  document = yaml::read<YamlDocument>(text);
    const auto& crafts   = document.synth_recipes;

    const std::array<const wire::Recipes*, kCraftSkills.size()> sections{
        &crafts.woodworking,
        &crafts.smithing,
        &crafts.goldsmithing,
        &crafts.clothcraft,
        &crafts.leathercraft,
        &crafts.bonecraft,
        &crafts.alchemy,
        &crafts.cooking,
    };

    Records                    records;
    std::unordered_set<uint32> ids;

    for (size_t craft = 0; craft < sections.size(); ++craft)
    {
        if (!*sections[craft])
        {
            continue;
        }

        for (const auto& [id, source] : **sections[craft])
        {
            const auto context = fmt::format("synth recipe {}", id);
            if (!ids.insert(id).second)
            {
                throw std::runtime_error(fmt::format("duplicate {}", context));
            }

            if (source.level == 0)
            {
                throw std::runtime_error(fmt::format("{} needs a level", context));
            }

            SynthRecipeData record{
                .Id      = id,
                .Name    = source.name,
                .Desynth = source.desynth.value_or(false),
                .KeyItem = yaml::resolveEnum(source.key_item),
                .Content = yaml::resolveEnum(source.content),
                .Crystal = yaml::resolveEnum(source.crystal),
                .Items   = shared::toRecipeItems(source.ingredients, source.results, context),
            };

            record.Skills[craft] = source.level;

            for (const auto& [skill, level] : yaml::resolveKeys(source.subcrafts))
            {
                const auto subcraft = std::ranges::find(kCraftSkills, skill);
                if (subcraft == kCraftSkills.end() || std::distance(kCraftSkills.begin(), subcraft) == static_cast<std::ptrdiff_t>(craft))
                {
                    throw std::runtime_error(fmt::format("{} has a subcraft that is not another craft", context));
                }

                if (level == 0 || level > source.level)
                {
                    throw std::runtime_error(fmt::format("{} needs subcraft levels from 1 to its own level, list it under the higher craft", context));
                }

                record.Skills[std::distance(kCraftSkills.begin(), subcraft)] = level;
            }

            if (record.Crystal == xi::Element::None)
            {
                throw std::runtime_error(fmt::format("{} needs a crystal element", context));
            }

            if (source.enabled.value_or(true))
            {
                records.emplace_back(std::move(record));
            }
        }
    }

    std::ranges::sort(records, {}, &SynthRecipeData::Id);
    return records;
}

} // namespace xi::data::datasets::synth_recipes
