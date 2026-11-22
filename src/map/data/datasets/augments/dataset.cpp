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

#include "data/datasets/augments/dataset.h"

#include "data/datasets/augments/yaml.h"
#include "data/yaml/read.h"

#include <fmt/format.h>

#include <stdexcept>
#include <utility>

namespace xi::data::datasets::augments
{

auto Dataset::decode(const std::string_view text) -> Records
{
    const auto document = yaml::read<YamlDocument>(text);

    Records records;
    for (const auto& [id, source] : document.augments)
    {
        if (id == 0 || id > kMaxAugmentId)
        {
            throw std::runtime_error(fmt::format("augment {} is outside 1 to {}", id, kMaxAugmentId));
        }

        AugmentData augment{
            .Mods       = yaml::resolveKeys(std::optional{ source.mods }),
            .Multiplier = source.multiplier.value_or(1),
        };

        if (source.pet)
        {
            augment.Pet = yaml::resolveEnum(*source.pet);
        }

        if (augment.Mods.empty())
        {
            throw std::runtime_error(fmt::format("augment {} has no mods", id));
        }

        if (augment.Mods.contains(xi::Mod::NONE))
        {
            throw std::runtime_error(fmt::format("augment {} lists mod none", id));
        }

        if (augment.Multiplier == 0)
        {
            throw std::runtime_error(fmt::format("augment {} has multiplier 0", id));
        }

        records.emplace(id, std::move(augment));
    }

    return records;
}

} // namespace xi::data::datasets::augments
