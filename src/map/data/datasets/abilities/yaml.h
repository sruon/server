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
#include "data/enums/ability_add_type.h"
#include "data/enums/ability_category.h"
#include "data/enums/action_target.h"
#include "data/enums/content.h"
#include "data/enums/merit.h"
#include "data/yaml/enum_token.h"
#include "data/yaml/schema_annotations.h"

#include <glaze/glaze.hpp>

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace xi::data::datasets::abilities::wire
{

struct Enmity
{
    int32 ce{};
    int32 ve{};
};

struct ChargeTier
{
    uint8                                     level{};
    uint8                                     max{};
    uint32                                    time{};
    std::optional<yaml::EnumToken<xi::Merit>> merit;
};

struct Charges
{
    std::optional<uint16>   recast_id;
    std::vector<ChargeTier> tiers;
};

struct Ability
{
    uint16                                                          id{};
    std::optional<bool>                                             enum_only;
    std::optional<uint8>                                            level;
    std::optional<std::vector<yaml::EnumToken<xi::ActionTarget>>>   target;
    std::optional<float>                                            range;
    std::optional<bool>                                             aoe;
    std::optional<uint8>                                            radius;
    std::optional<yaml::EnumToken<xi::AbilityCategory>>             category;
    std::optional<uint16>                                           animation;
    std::optional<uint16>                                           animation_time;
    std::optional<uint16>                                           cast_time;
    std::optional<uint16>                                           message;
    std::optional<uint16>                                           recast;
    std::optional<uint16>                                           recast_id;
    std::optional<Enmity>                                           enmity;
    std::optional<yaml::EnumToken<xi::Merit>>                       merit;
    std::optional<std::vector<yaml::EnumToken<xi::AbilityAddType>>> add_type;
    std::optional<yaml::EnumToken<xi::Content>>                     content;
    std::optional<Charges>                                          charges;
};

struct Document
{
    std::map<std::string, std::map<std::string, Ability>> abilities;

    using YamlRoot = yaml::DatasetRoot<&Document::abilities>;
};

} // namespace xi::data::datasets::abilities::wire

template <>
struct glz::json_schema<xi::data::datasets::abilities::wire::Enmity>
{
    glz::schema ce{ .description = "Cumulative enmity." };
    glz::schema ve{ .description = "Volatile enmity." };
};

template <>
struct glz::json_schema<xi::data::datasets::abilities::wire::ChargeTier>
{
    glz::schema level{ .description = "Level that grants this many charges.", .minimum = 1L, .maximum = 99L };
    glz::schema max{ .description = "Charges that can be stored.", .minimum = 1L };
    glz::schema time{ .description = "Seconds to restore one charge.", .minimum = 1L };
    glz::schema merit{ .description = "Merit that shortens the charge time." };
};

template <>
struct glz::json_schema<xi::data::datasets::abilities::wire::Charges>
{
    glz::schema recast_id{ .description = "Recast the charges apply to. Omit when it is this ability's own recast." };
    glz::schema tiers{ .description = "Charge tiers, lowest level first.", .minItems = 1UL };
};

template <>
struct glz::json_schema<xi::data::datasets::abilities::wire::Ability>
{
    glz::schema id{ .description = "Client-facing ability ID.", .minimum = 0L, .maximum = 1023L };
    glz::schema enum_only{ .description = "Reserves the ID for scripts and code without loading the ability.", .defaultValue = false };
    glz::schema level{ .description = "Level that grants the ability.", .defaultValue = 0, .minimum = 0L, .maximum = 99L };
    glz::schema target{ .description = "Who the ability can be used on. Defaults to the user only.", .minItems = 1UL, .uniqueItems = true };
    glz::schema range{ .description = "How far the target can be, in yalms.", .defaultValue = 0, .minimum = 0L };
    glz::schema aoe{ .description = "Hits everyone in radius around the user instead of a single target.", .defaultValue = false };
    glz::schema radius{ .description = "Area radius in yalms. Scripts can read it even when aoe is off.", .defaultValue = 0, .minimum = 0L };
    glz::schema category{ .description = "Action packet category.", .defaultValue = "ability_finish" };
    glz::schema animation{ .description = "Animation ID sent in the action packet.", .minimum = 0L, .maximum = 4095L };
    glz::schema animation_time{ .description = "Milliseconds the user is locked after the ability goes off.", .defaultValue = 2000, .minimum = 0L };
    glz::schema cast_time{ .description = "Milliseconds before the ability goes off.", .defaultValue = 0, .minimum = 0L };
    glz::schema message{ .description = "Default action message ID.", .defaultValue = 0, .minimum = 0L };
    glz::schema recast{ .description = "Recast in seconds.", .defaultValue = 0, .minimum = 0L };
    glz::schema recast_id{ .description = "Recast timer the ability uses.", .defaultValue = 0, .minimum = 0L };
    glz::schema enmity{ .description = "Enmity the ability generates." };
    glz::schema merit{ .description = "Merit that grants the ability or shortens its recast." };
    glz::schema add_type{ .description = "What else is needed before the ability is added.", .uniqueItems = true };
    glz::schema content{ .description = "Content tag that has to be enabled." };
    glz::schema charges{ .description = "Charges stored on the recast." };
};

template <>
struct glz::json_schema<xi::data::datasets::abilities::wire::Document>
{
    glz::schema abilities{ .description = "Abilities grouped by the job that learns them, then keyed by their canonical name. Abilities no job learns go under none." };
};
