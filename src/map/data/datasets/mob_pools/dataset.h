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
#include "common/types/hash_map.h"
#include "data/enums/mob_type.h"
#include "data/enums/roam_flag.h"
#include "data/enums/species.h"
#include "data/shared_types/mob_attributes/dataset.h"

#include <string>
#include <string_view>

namespace xi::data
{

struct MobPoolData
{
    uint32                 Id{};
    std::string            Name;
    std::string            DisplayName;
    xi::Species            Species{};
    xi::MobType            Type{};
    xi::RoamFlag           RoamFlags{};
    uint16                 SpellList{};
    uint16                 SkillList{};
    MobAttributesOverrides Attributes{};
};

using MobPools = HashMap<uint32, MobPoolData>;

} // namespace xi::data

namespace xi::data::datasets::mob_pools::wire
{

struct Document;

} // namespace xi::data::datasets::mob_pools::wire

namespace xi::data::datasets::mob_pools
{

struct Dataset
{
    using Records      = MobPools;
    using YamlDocument = wire::Document;

    static constexpr std::string_view kDataPath{ "mob_pools" };
    static constexpr std::string_view kTitle{ "Mob Pools" };
    static constexpr std::string_view kDescription{ "Mob templates for entities spawned outside zone files: instance mobs, allies, dynamic mobs, pets and trusts." };

    static auto decode(std::string_view text) -> Records;
};

} // namespace xi::data::datasets::mob_pools
