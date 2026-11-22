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
#include "data/enums/automaton_frame.h"

#include <string>
#include <string_view>
#include <vector>

namespace xi::data
{

struct AutomatonAbilityData
{
    uint16             Id{};
    std::string        Name{};
    xi::AutomatonFrame Frame{};
    uint16             SkillLevel{};
};

using AutomatonAbilities = std::vector<AutomatonAbilityData>; // ordered by id

} // namespace xi::data

namespace xi::data::datasets::automaton_abilities::wire
{

struct Document;

}

namespace xi::data::datasets::automaton_abilities
{

struct Dataset
{
    using Records      = AutomatonAbilities;
    using YamlDocument = wire::Document;

    static constexpr std::string_view kDataPath{ "automaton_abilities" };
    static constexpr std::string_view kTitle{ "Automaton Abilities" };
    static constexpr std::string_view kDescription{ "Automaton weapon skills and attachment abilities, with the frame and skill level they need." };

    static auto decode(std::string_view text) -> Records;
};

} // namespace xi::data::datasets::automaton_abilities
