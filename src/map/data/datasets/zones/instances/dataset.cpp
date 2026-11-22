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

#include "data/datasets/zones/instances/dataset.h"

#include "data/datasets/zones/instances/yaml.h"
#include "data/shared_types/position.h"
#include "data/yaml/read.h"

#include <fmt/format.h>

#include <set>
#include <stdexcept>
#include <string_view>

namespace xi::data::datasets::zones::instances
{

auto Dataset::decode(const std::string_view text) -> Records
{
    const auto document = yaml::read<YamlDocument>(text);

    Records          records;
    std::set<uint16> ids;
    records.reserve(document.instances.size());

    for (const auto& [name, source] : document.instances)
    {
        if (!ids.insert(source.id).second)
        {
            throw std::runtime_error(fmt::format("instance '{}' reuses id {}", name, source.id));
        }

        const auto music = source.music.value_or(wire::Music{});

        records.push_back(InstanceData{
            .Name      = name,
            .Id        = source.id,
            .Entrance  = yaml::resolveEnum(source.entrance),
            .Overlay   = source.overlay.value_or(0),
            .TimeLimit = std::chrono::minutes(source.time_limit.value_or(0)),
            .Entry     = shared::toPosition(source.entry, fmt::format("instance {}", name)),
            .Music     = {
                .Day         = music.day,
                .Night       = music.night,
                .BattleSolo  = music.battle_solo,
                .BattleParty = music.battle_party,
            },
        });
    }

    return records;
}

} // namespace xi::data::datasets::zones::instances
