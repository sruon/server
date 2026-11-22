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
#include "data/enums/fishing_nm_flag.h"
#include "data/yaml/enum_token.h"
#include "data/yaml/schema_annotations.h"

#include <glaze/glaze.hpp>

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace xi::data::datasets::zones::fishing_areas::wire
{

struct Cylinder
{
    std::vector<float> center;
    uint16             radius{};
    uint8              height{};
};

struct Poly
{
    float                           floor{};
    uint8                           height{};
    std::vector<std::vector<float>> corners;
};

struct Area
{
    uint8                                        id{};
    std::optional<Cylinder>                      cylinder;
    std::optional<Poly>                          poly;
    std::optional<std::map<std::string, uint16>> pool;
};

struct Monster
{
    std::vector<uint32>                                            spawns;
    std::optional<std::string>                                     area;
    uint8                                                          level{};
    uint8                                                          difficulty{};
    std::optional<uint8>                                           delay;
    std::optional<uint8>                                           move;
    std::optional<uint16>                                          ranking;
    std::optional<uint16>                                          rarity;
    std::optional<bool>                                            nm;
    std::optional<std::vector<yaml::EnumToken<xi::FishingNmFlag>>> nm_flags;
    std::optional<std::vector<uint16>>                             respawn;
    std::optional<std::string>                                     bait;
    std::optional<std::string>                                     alternative_bait;
    std::optional<uint8>                                           log;
    std::optional<uint8>                                           quest;
    std::optional<bool>                                            quest_only;
    std::optional<bool>                                            disabled;
};

struct Document
{
    std::optional<uint8>                          difficulty;
    std::optional<std::map<std::string, Area>>    areas;
    std::optional<std::map<std::string, Monster>> monsters;
};

} // namespace xi::data::datasets::zones::fishing_areas::wire

template <>
struct glz::json_schema<xi::data::datasets::zones::fishing_areas::wire::Cylinder>
{
    glz::schema center{ .description = "Center as x, y, z. Y is the bottom of the cylinder.", .minItems = 3UL, .maxItems = 3UL };
    glz::schema radius{ .description = "Radius, in yalms.", .minimum = 0L, .maximum = 255L };
    glz::schema height{ .description = "Height above the center, in yalms.", .minimum = 0L, .maximum = 255L };
};

template <>
struct glz::json_schema<xi::data::datasets::zones::fishing_areas::wire::Poly>
{
    glz::schema floor{ .description = "Y of the bottom of the area." };
    glz::schema height{ .description = "Height above the floor, in yalms.", .minimum = 0L, .maximum = 255L };
    glz::schema corners{ .description = "Corners as x, z, in order. The ring closes implicitly.", .minItems = 3UL };
};

template <>
struct glz::json_schema<xi::data::datasets::zones::fishing_areas::wire::Area>
{
    glz::schema id{ .description = "Area ID, passed to zone scripts. The first area a player stands in, by ID, wins.", .minimum = 1L, .maximum = 255L };
    glz::schema cylinder{ .description = "Cylinder the player must stand in. Omit both shapes for the whole zone." };
    glz::schema poly{ .description = "Polygon the player must stand in. Omit both shapes for the whole zone." };
    glz::schema pool{ .description = "Fish and items that bite here, keyed by fish name, with their hook weight from 0 to 1000." };
};

template <>
struct glz::json_schema<xi::data::datasets::zones::fishing_areas::wire::Monster>
{
    glz::schema spawns{ .description = "Mob spawn IDs from the zone's mobs.yaml, all sharing these values.", .minItems = 1UL, .uniqueItems = true };
    glz::schema area{ .description = "Fishing area it bites in. Omitted means every area of the zone." };
    glz::schema level{ .description = "Level it is rated at for the fight.", .minimum = 0L, .maximum = 255L };
    glz::schema difficulty{ .description = "Fight difficulty.", .minimum = 0L, .maximum = 255L };
    glz::schema delay{ .description = "Base arrow delay.", .defaultValue = 10, .minimum = 0L, .maximum = 255L };
    glz::schema move{ .description = "Base movement.", .defaultValue = 15, .minimum = 0L, .maximum = 255L };
    glz::schema ranking{ .description = "Ranking against the rod's rank.", .defaultValue = 10, .minimum = 0L, .maximum = 255L };
    glz::schema rarity{ .description = "Hook weight from 0 to 1000; higher is more common.", .defaultValue = 1000, .minimum = 0L, .maximum = 1000L };
    glz::schema nm{ .description = "Notorious monster.", .defaultValue = false };
    glz::schema nm_flags{ .description = "Fight behaviour of a notorious monster.", .uniqueItems = true };
    glz::schema respawn{ .description = "Minimum and maximum seconds before it can bite again.", .minItems = 2UL, .maxItems = 2UL };
    glz::schema bait{ .description = "Bait it needs, by bait name." };
    glz::schema alternative_bait{ .description = "Other bait it also takes, by bait name." };
    glz::schema log{ .description = "Quest log the quest field belongs to.", .defaultValue = 255, .minimum = 0L, .maximum = 255L };
    glz::schema quest{ .description = "Quest it belongs to.", .defaultValue = 255, .minimum = 0L, .maximum = 255L };
    glz::schema quest_only{ .description = "Only bites while its quest asks for it.", .defaultValue = false };
    glz::schema disabled{ .description = "Kept in the file but left out of the game.", .defaultValue = false };
};

template <>
struct glz::json_schema<xi::data::datasets::zones::fishing_areas::wire::Document>
{
    glz::schema difficulty{ .description = "Fishing difficulty of the zone.", .defaultValue = 0, .minimum = 0L, .maximum = 255L };
    glz::schema areas{ .description = "Fishing areas, keyed by name." };
    glz::schema monsters{ .description = "Monsters that can be fished up, keyed by name." };
};
