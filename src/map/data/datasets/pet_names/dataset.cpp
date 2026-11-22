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

#include "data/datasets/pet_names/dataset.h"

#include "data/datasets/pet_names/yaml.h"
#include "data/yaml/read.h"

#include <fmt/format.h>

#include <stdexcept>
#include <utility>

namespace xi::data::datasets::pet_names
{

auto Dataset::decode(const std::string_view text) -> Records
{
    auto document = yaml::read<YamlDocument>(text);

    for (const auto& [id, name] : document.pet_names)
    {
        if (id == 0 || name.empty())
        {
            throw std::runtime_error(fmt::format("pet name {} needs a non-zero id and a name", id));
        }
    }

    return std::move(document.pet_names);
}

} // namespace xi::data::datasets::pet_names
