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

#include "data/datasets/pets/dataset.h"

#include "data/datasets/pets/yaml.h"
#include "data/yaml/read.h"

#include <fmt/format.h>

#include <algorithm>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace xi::data::datasets::pets
{

auto Dataset::decode(const std::string_view text) -> Records
{
    const auto document = yaml::read<YamlDocument>(text);

    Records                   records;
    std::unordered_set<uint8> ids;

    for (const auto& [key, source] : document.pets)
    {
        yaml::verifyNamedMapEntry<xi::PetId>(key, source.id);
        if (!ids.insert(source.id).second)
        {
            throw std::runtime_error(fmt::format("duplicate pet id {}", source.id));
        }

        if (source.name.empty())
        {
            throw std::runtime_error(fmt::format("{} has no name", key));
        }

        PetData record{
            .Id         = static_cast<xi::PetId>(source.id),
            .Name       = source.name,
            .Pool       = source.pool,
            .MinLevel   = source.min_level.value_or(1),
            .MaxLevel   = source.max_level.value_or(99),
            .Duration   = std::chrono::seconds(source.duration.value_or(0)),
            .Element    = yaml::resolveEnum(source.element),
            .DamageType = yaml::resolveEnum(source.damage_type),
        };

        if (record.MinLevel > record.MaxLevel)
        {
            throw std::runtime_error(fmt::format("{} min_level is above max_level", key));
        }

        records.emplace_back(std::move(record));
    }

    std::ranges::sort(records, {}, &PetData::Id);
    return records;
}

} // namespace xi::data::datasets::pets
