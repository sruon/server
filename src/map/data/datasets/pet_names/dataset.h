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

#pragma once

#include "common/cbasetypes.h"

#include <map>
#include <string>
#include <string_view>

namespace xi::data
{

using PetNames = std::map<uint16, std::string>; // keyed by the client name id

} // namespace xi::data

namespace xi::data::datasets::pet_names::wire
{

struct Document;

}

namespace xi::data::datasets::pet_names
{

struct Dataset
{
    using Records      = PetNames;
    using YamlDocument = wire::Document;

    static constexpr std::string_view kDataPath{ "pet_names" };
    static constexpr std::string_view kTitle{ "Pet Names" };
    static constexpr std::string_view kDescription{ "Names a player can give a wyvern, an automaton or a chocobo." };

    static auto decode(std::string_view text) -> Records;
};

} // namespace xi::data::datasets::pet_names
