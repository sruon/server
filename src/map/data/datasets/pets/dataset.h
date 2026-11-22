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
#include "data/enums/damage_type.h"
#include "data/enums/element.h"
#include "data/enums/pet_id.h"

#include <chrono>
#include <string>
#include <string_view>
#include <vector>

namespace xi::data
{

struct PetData
{
    xi::PetId            Id{};
    std::string          Name{};
    uint32               Pool{};
    uint8                MinLevel{};
    uint8                MaxLevel{};
    std::chrono::seconds Duration{}; // zero when the pet stays until dismissed
    xi::Element          Element{};
    xi::DamageType       DamageType{};
};

using Pets = std::vector<PetData>; // ordered by id

} // namespace xi::data

namespace xi::data::datasets::pets::wire
{

struct Document;

}

namespace xi::data::datasets::pets
{

struct Dataset
{
    using Records      = Pets;
    using YamlDocument = wire::Document;

    static constexpr std::string_view kDataPath{ "pets" };
    static constexpr std::string_view kTitle{ "Pets" };
    static constexpr std::string_view kDescription{ "Pets a player can call, the mob pool each one uses, and how long it stays." };

    static auto decode(std::string_view text) -> Records;
};

} // namespace xi::data::datasets::pets
