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
#include <unordered_set>
#include <utility>

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

        records.emplace_back(BattlefieldData{
            .Name  = name,
            .Index = source.index,
        });
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
