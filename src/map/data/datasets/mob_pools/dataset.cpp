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

#include "data/datasets/mob_pools/dataset.h"

#include "data/datasets/mob_pools/yaml.h"
#include "data/yaml/read.h"

#include <fmt/format.h>

#include <stdexcept>

namespace xi::data::datasets::mob_pools
{

auto Dataset::decode(const std::string_view text) -> Records
{
    const auto document = yaml::read<YamlDocument>(text);

    Records records;
    for (const auto& [name, source] : document.mob_pools)
    {
        if (source.id == 0)
        {
            throw std::runtime_error(fmt::format("mob pool '{}' declares reserved pool id 0", name));
        }

        MobPoolData record{
            .Id          = source.id,
            .Name        = name,
            .DisplayName = source.display_name.value_or(name),
            .Species     = yaml::resolveEnum(source.species),
            .Type        = yaml::resolveFlags(source.type),
            .RoamFlags   = yaml::resolveFlags(source.roam),
            .SpellList   = source.spell_list_id.value_or(0),
            .SkillList   = source.skill_list_id.value_or(0),
            .Attributes  = convertAttributes(source.attributes, name),
        };

        if (!records.try_emplace(source.id, std::move(record)).second)
        {
            throw std::runtime_error(fmt::format("mob pool '{}' reuses pool id {}", name, source.id));
        }
    }

    return records;
}

} // namespace xi::data::datasets::mob_pools
