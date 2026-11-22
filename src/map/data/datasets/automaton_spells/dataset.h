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
#include "data/enums/automaton_head.h"
#include "data/enums/immunity.h"
#include "data/enums/status_effect.h"

#include <string>
#include <string_view>
#include <vector>

namespace xi::data
{

struct AutomatonSpellData
{
    uint16                         Id{};
    std::string                    Name{};
    uint16                         SkillLevel{};
    std::vector<xi::AutomatonHead> Heads{};
    xi::StatusEffect               Enfeeble{};
    xi::Immunity                   Immunity{};
    std::vector<xi::StatusEffect>  Removes{};
};

using AutomatonSpells = std::vector<AutomatonSpellData>; // ordered by id

} // namespace xi::data

namespace xi::data::datasets::automaton_spells::wire
{

struct Document;

}

namespace xi::data::datasets::automaton_spells
{

struct Dataset
{
    using Records      = AutomatonSpells;
    using YamlDocument = wire::Document;

    static constexpr std::string_view kDataPath{ "automaton_spells" };
    static constexpr std::string_view kTitle{ "Automaton Spells" };
    static constexpr std::string_view kDescription{ "Spells automatons cast, the heads that cast them and the skill they need." };

    static auto decode(std::string_view text) -> Records;
};

} // namespace xi::data::datasets::automaton_spells
