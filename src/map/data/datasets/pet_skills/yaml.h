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
#include "data/enums/skill_flag.h"
#include "data/enums/skillchain_type.h"
#include "data/enums/target_type.h"
#include "data/shared_types/action_area/yaml.h"
#include "data/yaml/enum_token.h"
#include "data/yaml/schema_annotations.h"

#include <glaze/glaze.hpp>

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace xi::data::datasets::pet_skills::wire
{

struct Area
{
    std::optional<shared::AoeSphere> sphere;
    std::optional<shared::AoeCone>   cone;
};

struct PetSkill
{
    uint16                                                          id{};
    std::optional<std::string>                                      name;
    std::optional<uint16>                                           mob_skill;
    uint16                                                          animation{};
    std::optional<float>                                            range;
    std::optional<std::vector<yaml::EnumToken<xi::TargetType>>>     target;
    std::optional<Area>                                             aoe;
    std::optional<float>                                            master_range;
    std::optional<uint16>                                           animation_time;
    std::optional<uint16>                                           prepare_time;
    std::optional<uint16>                                           message;
    std::optional<std::vector<yaml::EnumToken<xi::SkillFlag>>>      flags;
    std::optional<int16>                                            param;
    std::optional<uint8>                                            finish_category;
    std::optional<uint8>                                            knockback;
    std::optional<std::vector<yaml::EnumToken<xi::SkillchainType>>> skillchain;
};

struct Document
{
    std::map<std::string, std::map<std::string, PetSkill>> pet_skills;

    using YamlRoot = yaml::DatasetRoot<&Document::pet_skills>;
};

} // namespace xi::data::datasets::pet_skills::wire

template <>
struct glz::json_schema<xi::data::datasets::pet_skills::wire::Area>
{
    glz::schema sphere{ .description = "Sphere around the caster or the target. Set this or cone, not both." };
    glz::schema cone{ .description = "Cone in front of the pet. Set this or sphere, not both." };
};

template <>
struct glz::json_schema<xi::data::datasets::pet_skills::wire::PetSkill>
{
    glz::schema id{ .description = "Pet skill ID, the same as the player ability that orders it.", .minimum = 0L, .maximum = 65535L };
    glz::schema name{ .description = "Script name under scripts/actions/abilities/pets when it is not the key." };
    glz::schema mob_skill{ .description = "Mob skill the client shows for jug pet and luopan skills.", .minimum = 0L, .maximum = 65535L };
    glz::schema animation{ .description = "Animation ID sent in the action packet.", .minimum = 0L, .maximum = 65535L };
    glz::schema range{ .description = "How far the target can be, in yalms, on top of both hitboxes.", .defaultValue = 3, .minimum = 0L };
    glz::schema target{ .description = "Who the skill can be used on. Omit for an enemy.", .minItems = 1UL, .uniqueItems = true };
    glz::schema aoe{ .description = "Area the skill reaches. Omit for a single target." };
    glz::schema master_range{ .description = "Single-target skill a jug pet uses on itself that can target the party: how close, in yalms, the master must be to share it.", .minimum = 0L };
    glz::schema animation_time{ .description = "Animation time in milliseconds.", .defaultValue = 2000, .minimum = 0L, .maximum = 65535L };
    glz::schema prepare_time{ .description = "Readying time in milliseconds.", .defaultValue = 1000, .minimum = 0L, .maximum = 65535L };
    glz::schema message{ .description = "Basic message ID shown when the skill lands.", .defaultValue = 0, .minimum = 0L, .maximum = 65535L };
    glz::schema flags{ .description = "Skill behaviour flags.", .uniqueItems = true };
    glz::schema param{ .description = "Free value scripts read with getParam.", .defaultValue = 0 };
    glz::schema finish_category{ .description = "Action category of the finish packet. Omit for 11 (mob skill) with a mob_skill and 13 (pet skill) without.", .minimum = 0L, .maximum = 15L };
    glz::schema knockback{ .description = "Knockback strength.", .defaultValue = 0, .minimum = 0L, .maximum = 255L };
    glz::schema skillchain{ .description = "Skillchain properties, highest priority first.", .maxItems = 3UL, .uniqueItems = true };
};

template <>
struct glz::json_schema<xi::data::datasets::pet_skills::wire::Document>
{
    glz::schema pet_skills{ .description = "Pet skills grouped by the pet or kind of pet that uses them, then keyed by canonical name." };
};
