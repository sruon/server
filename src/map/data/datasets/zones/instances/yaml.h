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
#include "data/enums/zone.h"
#include "data/yaml/enum_token.h"
#include "data/yaml/schema_annotations.h"

#include <glaze/glaze.hpp>

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace xi::data::datasets::zones::instances::wire
{

struct Music
{
    std::optional<uint16> day;
    std::optional<uint16> night;
    std::optional<uint16> battle_solo;
    std::optional<uint16> battle_party;
};

struct Instance
{
    uint16                      id{};
    yaml::EnumToken<xi::ZoneId> entrance;
    std::optional<uint16>       overlay;
    std::optional<uint16>       time_limit;
    std::vector<float>          entry;
    std::optional<Music>        music;
};

struct Document
{
    std::map<std::string, Instance> instances;

    using YamlRoot = yaml::DatasetRoot<&Document::instances>;
};

} // namespace xi::data::datasets::zones::instances::wire

template <>
struct glz::json_schema<xi::data::datasets::zones::instances::wire::Music>
{
    glz::schema day{ .description = "Background music by day. Omitted keeps the zone's." };
    glz::schema night{ .description = "Background music by night. Omitted keeps the zone's." };
    glz::schema battle_solo{ .description = "Solo battle music. Omitted keeps the zone's." };
    glz::schema battle_party{ .description = "Party battle music. Omitted keeps the zone's." };
};

template <>
struct glz::json_schema<xi::data::datasets::zones::instances::wire::Instance>
{
    glz::schema id{ .description = "Instance id scripts and createInstance use. By convention the zone id times 100 plus an offset." };
    glz::schema entrance{ .description = "Zone players return to when they leave." };
    glz::schema overlay{ .description = "Alternative entity list the client loads in place of the zone's own. Omitted means none." };
    glz::schema time_limit{ .description = "Minutes the instance lasts. Defaults to 0, meaning no limit." };
    glz::schema entry{ .description = "Where players enter, as x, y, z and optionally a facing of 0-255." };
    glz::schema music{ .description = "Music overriding the zone's while inside." };
};

template <>
struct glz::json_schema<xi::data::datasets::zones::instances::wire::Document>
{
    glz::schema instances{ .description = "Instances keyed by name, which names their script. A mob spawn or NPC joins one by naming it in its instance field." };
};
