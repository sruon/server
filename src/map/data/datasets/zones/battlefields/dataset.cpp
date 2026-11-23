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

#include "data/datasets/zones/battlefields/dataset.h"

#include "data/datasets/zones/battlefields/yaml.h"
#include "data/yaml/enum_token.h"
#include "data/yaml/read.h"

#include <fmt/format.h>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <variant>

namespace xi::data::datasets::zones::battlefields
{

auto Dataset::decode(const std::string_view text) -> Records
{
    const auto document = yaml::read<YamlDocument>(text);

    Records                   records;
    std::unordered_set<uint8> indices;

    for (const auto& [name, source] : document.battlefields)
    {
        if (source.index >= (1 << kBattlefieldIndexBits))
        {
            throw std::runtime_error(fmt::format("battlefield '{}' has index {}, past the entry menu", name, source.index));
        }

        if (!indices.insert(source.index).second)
        {
            throw std::runtime_error(fmt::format("duplicate battlefield index {}", source.index));
        }

        BattlefieldData record{
            .Name  = name,
            .Index = source.index,
        };

        if (source.time_limit)
        {
            record.TimeLimit = std::chrono::minutes(*source.time_limit);
        }

        if (source.delay_to_exit)
        {
            record.DelayToExit = std::chrono::seconds(*source.delay_to_exit);
        }

        if (source.level_cap)
        {
            if (const auto* level = std::get_if<uint8>(&*source.level_cap))
            {
                record.LevelCap = *level;
            }
            else if (std::get<wire::LevelCapKeyword>(*source.level_cap).Value == "max")
            {
                record.CapAtMaxLevel = true;
            }
            else
            {
                throw std::runtime_error(fmt::format("battlefield '{}' level_cap must be a level or max", name));
            }
        }

        record.MaxPlayers     = source.max_players.value_or(record.MaxPlayers);
        record.CanLoseExp     = source.can_lose_exp.value_or(record.CanLoseExp);
        record.AllowTrusts    = source.allow_trusts.value_or(record.AllowTrusts);
        record.IsMission      = source.is_mission.value_or(record.IsMission);
        record.AllowSubjob    = source.allow_subjob.value_or(record.AllowSubjob);
        record.CleanupDebuffs = source.cleanup_debuffs.value_or(record.CleanupDebuffs);
        record.HasWipeGrace   = source.has_wipe_grace.value_or(record.HasWipeGrace);
        record.ShowTimer      = source.show_timer.value_or(record.ShowTimer);

        records.emplace_back(std::move(record));
    }

    std::ranges::sort(records, {}, &BattlefieldData::Index);
    return records;
}

void Dataset::verifyZone(const Records& records, const xi::ZoneId zoneId)
{
    for (const auto& record : records)
    {
        yaml::verifyNamedMapEntry<xi::BattlefieldId>(record.Name, std::to_underlying(battlefieldId(zoneId, record.Index)));
    }
}

} // namespace xi::data::datasets::zones::battlefields
