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

#include "data/datasets/job_points/dataset.h"

#include "data/datasets/job_points/yaml.h"
#include "data/yaml/enum_keyed_map.h"
#include "data/yaml/enum_token.h"
#include "data/yaml/read.h"

#include <fmt/format.h>

#include <algorithm>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace xi::data::datasets::job_points
{

namespace
{

constexpr uint16 kCategoriesPerJob  = 10;
constexpr uint16 kMaxJobPointsSpent = 2100; // 10 categories at rank 20

} // namespace

auto Dataset::decode(const std::string_view text) -> Records
{
    const auto source = yaml::read<YamlDocument>(text).job_points;

    Records records;
    for (const auto& [job, entry] : yaml::resolveKeys(std::optional{ source }))
    {
        if (job == xi::Job::NONE || job > xi::Job::RUN)
        {
            throw std::runtime_error(fmt::format("job {} has no job points", EnumTraits<xi::Job>::toName(job)));
        }

        auto& record = records[job];
        for (const auto& [name, category] : entry.categories)
        {
            yaml::verifyNamedMapEntry<xi::JobPoint>(name, category.id);
            if (category.id / 32 != static_cast<uint16>(job) || category.id % 32 >= kCategoriesPerJob)
            {
                throw std::runtime_error(fmt::format("job point {} id {:#x} is outside {}'s range", name, category.id, EnumTraits<xi::Job>::toName(job)));
            }

            record.Categories.emplace_back(static_cast<xi::JobPoint>(category.id));
        }

        std::ranges::sort(record.Categories);
        if (std::ranges::adjacent_find(record.Categories) != record.Categories.end())
        {
            throw std::runtime_error(fmt::format("duplicate job point id in {}", EnumTraits<xi::Job>::toName(job)));
        }

        std::set<std::string> spells;
        for (const auto& [spent, tier] : entry.gifts.value_or(wire::Gifts{}))
        {
            if (spent == 0 || spent > kMaxJobPointsSpent)
            {
                throw std::runtime_error(fmt::format("{} gift at {} job points is outside 1-{}", EnumTraits<xi::Job>::toName(job), spent, kMaxJobPointsSpent));
            }

            std::set<xi::Mod> mods;
            for (const auto& [key, value] : tier.Entries)
            {
                if (key == wire::kSpellsKey)
                {
                    const auto* names = std::get_if<std::vector<std::string>>(&value);
                    if (!names)
                    {
                        throw std::runtime_error(fmt::format("{} gift spells at {} must be a list", EnumTraits<xi::Job>::toName(job), spent));
                    }

                    for (const auto& name : *names)
                    {
                        if (!spells.insert(name).second)
                        {
                            throw std::runtime_error(fmt::format("duplicate gift spell {} in {}", name, EnumTraits<xi::Job>::toName(job)));
                        }

                        record.GiftSpells.push_back({ .JobPointsSpent = spent, .Spell = name });
                    }

                    continue;
                }

                const auto* amount = std::get_if<int16>(&value);
                if (!amount)
                {
                    throw std::runtime_error(fmt::format("{} gift {} at {} must be a number", EnumTraits<xi::Job>::toName(job), key, spent));
                }

                const auto mod = EnumTraits<xi::Mod>::fromName(key);
                if (!mods.insert(mod).second)
                {
                    throw std::runtime_error(fmt::format("duplicate gift {} at {} in {}", key, spent, EnumTraits<xi::Job>::toName(job)));
                }

                record.Gifts.push_back({ .JobPointsSpent = spent, .Mod = mod, .Value = *amount });
            }
        }
    }

    return records;
}

} // namespace xi::data::datasets::job_points
