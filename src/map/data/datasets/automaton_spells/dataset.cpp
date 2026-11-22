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

#include "data/datasets/automaton_spells/dataset.h"

#include "data/datasets/automaton_spells/yaml.h"
#include "data/yaml/read.h"

#include <fmt/format.h>

#include <algorithm>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace xi::data::datasets::automaton_spells
{

auto Dataset::decode(const std::string_view text) -> Records
{
    const auto document = yaml::read<YamlDocument>(text);

    Records                    records;
    std::unordered_set<uint16> ids;

    for (const auto& [name, source] : document.automaton_spells)
    {
        if (!ids.insert(source.id).second)
        {
            throw std::runtime_error(fmt::format("duplicate automaton spell id {}", source.id));
        }

        if (source.heads.empty())
        {
            throw std::runtime_error(fmt::format("{} has no heads", name));
        }

        AutomatonSpellData record{
            .Id         = source.id,
            .Name       = name,
            .SkillLevel = source.skill_level.value_or(0),
            .Enfeeble   = yaml::resolveEnum(source.enfeeble),
            .Immunity   = yaml::resolveEnum(source.immunity),
        };

        for (const auto& head : source.heads)
        {
            record.Heads.emplace_back(yaml::resolveEnum(head));
        }

        for (const auto& effect : source.removes.value_or(std::vector<yaml::EnumToken<xi::StatusEffect>>{}))
        {
            record.Removes.emplace_back(yaml::resolveEnum(effect));
        }

        records.emplace_back(std::move(record));
    }

    std::ranges::sort(records, {}, &AutomatonSpellData::Id);
    return records;
}

} // namespace xi::data::datasets::automaton_spells
