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

#include <string>
#include <string_view>
#include <vector>

namespace xi::data
{

struct MobSpellData
{
    std::string Name{};
    uint8       MinLevel{};
    uint8       MaxLevel{};
};

struct MobSpellListData
{
    uint16                    Id{};
    std::string               Name{};
    std::vector<MobSpellData> Spells{};
};

using MobSpellLists = std::vector<MobSpellListData>; // ordered by id

} // namespace xi::data

namespace xi::data::datasets::mob_spell_lists::wire
{

struct Document;

}

namespace xi::data::datasets::mob_spell_lists
{

struct Dataset
{
    using Records      = MobSpellLists;
    using YamlDocument = wire::Document;

    static constexpr std::string_view kDataPath{ "mob_spell_lists" };
    static constexpr std::string_view kTitle{ "Mob Spell Lists" };
    static constexpr std::string_view kDescription{ "Spells mobs, pets and trusts cast, and the levels they cast them at." };

    static auto decode(std::string_view text) -> Records;
};

} // namespace xi::data::datasets::mob_spell_lists
