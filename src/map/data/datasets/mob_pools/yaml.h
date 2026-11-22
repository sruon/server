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
#include "data/enums/mob_type.h"
#include "data/enums/roam_flag.h"
#include "data/enums/species.h"
#include "data/shared_types/mob_attributes/yaml.h"
#include "data/yaml/enum_token.h"
#include "data/yaml/schema_annotations.h"

#include <glaze/glaze.hpp>

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace xi::data::datasets::mob_pools::wire
{

struct Pool
{
    uint32                                                    id{};
    std::optional<std::string>                                display_name;
    yaml::EnumToken<xi::Species>                              species;
    std::optional<std::vector<yaml::EnumToken<xi::MobType>>>  type;
    std::optional<std::vector<yaml::EnumToken<xi::RoamFlag>>> roam;
    std::optional<uint16>                                     skill_list_id;
    std::optional<uint16>                                     spell_list_id;
    std::optional<shared::MobAttributes>                      attributes;
};

struct Document
{
    std::map<std::string, Pool> mob_pools;

    using YamlRoot = yaml::DatasetRoot<&Document::mob_pools>;
};

} // namespace xi::data::datasets::mob_pools::wire

template <>
struct glz::json_schema<xi::data::datasets::mob_pools::wire::Pool>
{
    glz::schema id{ .description = "Pool id that instances, mob groups, pets and trusts refer to. Trusts use their spell id plus 5000.", .minimum = 1L };
    glz::schema display_name{ .description = "Name sent to the client. Defaults to the key." };
    glz::schema species{ .description = "Species from ecosystems.yaml. Its attributes apply under the pool's own." };
    glz::schema type{ .description = "Mob type flags. Defaults to normal.", .uniqueItems = true };
    glz::schema roam{ .description = "Roam flags. Defaults to none. Only instance mobs read them.", .uniqueItems = true };
    glz::schema skill_list_id{ .description = "Mob skill list. Defaults to 0, no skills." };
    glz::schema spell_list_id{ .description = "Mob spell list. Defaults to 0, no spells." };
    glz::schema attributes{ .description = "Attributes applied over the species chain. Jobs default to war/war." };
};

template <>
struct glz::json_schema<xi::data::datasets::mob_pools::wire::Document>
{
    glz::schema mob_pools{ .description = "Mob pools keyed by name. A name shared by several pools carries the pool id as a suffix." };
};
