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

#include "data/datasets/mob_spell_lists/dataset.h"

#include "data/datasets/mob_spell_lists/yaml.h"
#include "data/enums/mob_spell_list.h"
#include "data/yaml/enum_token.h"
#include "data/yaml/read.h"

#include <fmt/format.h>

#include <algorithm>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace xi::data::datasets::mob_spell_lists
{

auto Dataset::decode(const std::string_view text) -> Records
{
    const auto document = yaml::read<YamlDocument>(text);

    Records                    records;
    std::unordered_set<uint16> ids;

    for (const auto& [name, source] : document.mob_spell_lists)
    {
        yaml::verifyNamedMapEntry<xi::MobSpellList>(name, source.id);
        if (source.id == 0)
        {
            throw std::runtime_error(fmt::format("{} uses spell list id 0, which is reserved for no spells", name));
        }

        if (!ids.insert(source.id).second)
        {
            throw std::runtime_error(fmt::format("duplicate mob spell list id {}", source.id));
        }

        MobSpellListData record{
            .Id   = source.id,
            .Name = name,
        };

        for (const auto& [spellName, spell] : source.spells)
        {
            const MobSpellData entry{
                .Name     = spellName,
                .MinLevel = spell.min_level.value_or(1),
                .MaxLevel = spell.max_level.value_or(255),
            };

            if (entry.MinLevel > entry.MaxLevel)
            {
                throw std::runtime_error(fmt::format("{} casts {} from level {} to {}", name, spellName, entry.MinLevel, entry.MaxLevel));
            }

            record.Spells.emplace_back(entry);
        }

        records.emplace_back(std::move(record));
    }

    std::ranges::sort(records, {}, &MobSpellListData::Id);
    return records;
}

} // namespace xi::data::datasets::mob_spell_lists
