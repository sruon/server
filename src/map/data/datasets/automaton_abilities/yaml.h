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
#include "data/yaml/enum_token.h"
#include "data/yaml/schema_annotations.h"

#include <glaze/glaze.hpp>

#include <map>
#include <optional>
#include <string>

namespace xi::data::datasets::automaton_abilities::wire
{

struct AutomatonAbility
{
    uint16                                             id{};
    std::optional<yaml::EnumToken<xi::AutomatonFrame>> frame;
    std::optional<uint16>                              skill_level;
};

struct Document
{
    std::map<std::string, AutomatonAbility> automaton_abilities;

    using YamlRoot = yaml::DatasetRoot<&Document::automaton_abilities>;
};

} // namespace xi::data::datasets::automaton_abilities::wire

template <>
struct glz::json_schema<xi::data::datasets::automaton_abilities::wire::AutomatonAbility>
{
    glz::schema id{ .description = "Mob skill ID the automaton uses.", .minimum = 1L, .maximum = 65535L };
    glz::schema frame{ .description = "Frame the ability needs. Omit when any frame can use it." };
    glz::schema skill_level{ .description = "Automaton melee, ranged or magic skill the ability needs.", .defaultValue = 0, .minimum = 0L, .maximum = 65535L };
};

template <>
struct glz::json_schema<xi::data::datasets::automaton_abilities::wire::Document>
{
    glz::schema automaton_abilities{ .description = "Automaton abilities keyed by their script name under scripts/actions/abilities/pets/automaton, in ID order." };
};
