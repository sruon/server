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
#include "data/yaml/enum_token.h"
#include "data/yaml/schema_annotations.h"

#include <glaze/glaze.hpp>

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace xi::data::datasets::automaton_spells::wire
{

struct AutomatonSpell
{
    uint16                                                        id{};
    std::optional<uint16>                                         skill_level;
    std::vector<yaml::EnumToken<xi::AutomatonHead>>               heads;
    std::optional<yaml::EnumToken<xi::StatusEffect>>              enfeeble;
    std::optional<yaml::EnumToken<xi::Immunity>>                  immunity;
    std::optional<std::vector<yaml::EnumToken<xi::StatusEffect>>> removes;
};

struct Document
{
    std::map<std::string, AutomatonSpell> automaton_spells;

    using YamlRoot = yaml::DatasetRoot<&Document::automaton_spells>;
};

} // namespace xi::data::datasets::automaton_spells::wire

template <>
struct glz::json_schema<xi::data::datasets::automaton_spells::wire::AutomatonSpell>
{
    glz::schema id{ .description = "Spell ID.", .minimum = 1L, .maximum = 65535L };
    glz::schema skill_level{ .description = "Automaton magic skill the spell needs.", .defaultValue = 0, .minimum = 0L, .maximum = 65535L };
    glz::schema heads{ .description = "Heads that can cast the spell.", .minItems = 1UL, .uniqueItems = true };
    glz::schema enfeeble{ .description = "Effect the spell inflicts. The automaton skips the spell while the target has it." };
    glz::schema immunity{ .description = "Immunity that makes the automaton skip the spell." };
    glz::schema removes{ .description = "Effects the spell removes. The automaton casts it to remove them.", .uniqueItems = true };
};

template <>
struct glz::json_schema<xi::data::datasets::automaton_spells::wire::Document>
{
    glz::schema automaton_spells{ .description = "Automaton spells keyed by spell name, in ID order." };
};
