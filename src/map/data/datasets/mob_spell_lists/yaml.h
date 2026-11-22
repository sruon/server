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
#include "data/yaml/schema_annotations.h"

#include <glaze/glaze.hpp>

#include <map>
#include <optional>
#include <string>

namespace xi::data::datasets::mob_spell_lists::wire
{

struct MobSpell
{
    std::optional<uint8> min_level;
    std::optional<uint8> max_level;
};

struct MobSpellList
{
    uint16                          id{};
    std::map<std::string, MobSpell> spells;
};

struct Document
{
    std::map<std::string, MobSpellList> mob_spell_lists;

    using YamlRoot = yaml::DatasetRoot<&Document::mob_spell_lists>;
};

} // namespace xi::data::datasets::mob_spell_lists::wire

template <>
struct glz::json_schema<xi::data::datasets::mob_spell_lists::wire::MobSpell>
{
    glz::schema min_level{ .description = "Lowest caster level that casts the spell.", .defaultValue = 1, .minimum = 0L, .maximum = 255L };
    glz::schema max_level{ .description = "Highest caster level that casts the spell.", .defaultValue = 255, .minimum = 0L, .maximum = 255L };
};

template <>
struct glz::json_schema<xi::data::datasets::mob_spell_lists::wire::MobSpellList>
{
    glz::schema id{ .description = "Spell list ID that mobs, pets and trusts reference.", .minimum = 1L, .maximum = 65535L };
    glz::schema spells{ .description = "Spells in the list, keyed by spell name." };
};

template <>
struct glz::json_schema<xi::data::datasets::mob_spell_lists::wire::Document>
{
    glz::schema mob_spell_lists{ .description = "Mob spell lists keyed by their canonical name, in ID order." };
};
