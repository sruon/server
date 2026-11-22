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
#include "data/enums/mob_skill_aoe.h"
#include "data/enums/mob_skill_flag.h"
#include "data/enums/skillchain_type.h"
#include "data/enums/target_type.h"
#include "data/yaml/enum_token.h"
#include "data/yaml/schema_annotations.h"

#include <glaze/glaze.hpp>

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace xi::data::datasets::mob_skills::wire
{

struct MobSkill
{
    uint16                                                          id{};
    std::optional<std::string>                                      name;
    uint16                                                          animation{};
    std::optional<yaml::EnumToken<xi::MobSkillAoe>>                 aoe;
    std::optional<float>                                            radius;
    std::optional<float>                                            distance;
    std::optional<uint32>                                           animation_time;
    std::optional<uint32>                                           ready_time;
    std::optional<std::vector<yaml::EnumToken<xi::TargetType>>>     target;
    std::optional<std::vector<yaml::EnumToken<xi::MobSkillFlag>>>   flags;
    std::optional<int16>                                            param;
    std::optional<uint8>                                            knockback;
    std::optional<std::vector<yaml::EnumToken<xi::SkillchainType>>> skillchain;
};

struct MobSkillList
{
    uint16                                  id{};
    std::optional<std::vector<std::string>> skills;
    std::optional<std::vector<uint16>>      abilities;
};

struct DocumentRoot;

struct Document
{
    std::map<std::string, MobSkill>     mob_skills;
    std::map<std::string, MobSkillList> mob_skill_lists;

    using YamlRoot = DocumentRoot;
};

struct DocumentRoot
{
    static constexpr auto value = glz::object(
        "meta",
        glz::skip{},
        "mob_skills",
        &Document::mob_skills,
        "mob_skill_lists",
        &Document::mob_skill_lists);

    static constexpr auto requires_key(const std::string_view key, const bool isNullable) -> bool
    {
        return key != "meta" && !isNullable;
    }
};

} // namespace xi::data::datasets::mob_skills::wire

template <>
struct glz::json_schema<xi::data::datasets::mob_skills::wire::MobSkill>
{
    glz::schema id{ .description = "Mob skill ID sent in the action packet.", .minimum = 1L, .maximum = 65535L };
    glz::schema name{ .description = "Script name under scripts/actions/mobskills. Defaults to the key." };
    glz::schema animation{ .description = "Animation ID sent in the action packet.", .minimum = 0L, .maximum = 65535L };
    glz::schema aoe{ .description = "Area the skill reaches.", .defaultValue = "single" };
    glz::schema radius{ .description = "Area radius in yalms. 0 uses the distance for an area around the target.", .defaultValue = 0, .minimum = 0L };
    glz::schema distance{ .description = "How far the target can be, in yalms.", .defaultValue = 7, .minimum = 0L };
    glz::schema animation_time{ .description = "Milliseconds the animation lasts.", .defaultValue = 2000, .minimum = 0L, .maximum = 65535L };
    glz::schema ready_time{ .description = "Milliseconds from readying to using the skill.", .defaultValue = 1500, .minimum = 0L, .maximum = 65535L };
    glz::schema target{ .description = "Who the skill can be used on. Defaults to [enemy].", .minItems = 1UL, .uniqueItems = true };
    glz::schema flags{ .description = "Behaviour flags.", .uniqueItems = true };
    glz::schema param{ .description = "Free parameter read by the skill script.", .defaultValue = 0, .minimum = -32768L, .maximum = 32767L };
    glz::schema knockback{ .description = "Knockback level.", .defaultValue = 0, .minimum = 0L, .maximum = 7L };
    glz::schema skillchain{ .description = "Skillchain properties, highest priority first.", .maxItems = 3UL, .uniqueItems = true };
};

template <>
struct glz::json_schema<xi::data::datasets::mob_skills::wire::MobSkillList>
{
    glz::schema id{ .description = "Skill list ID set on mob pools and by the SKILL_LIST mob mod.", .minimum = 1L, .maximum = 65535L };
    glz::schema skills{ .description = "Mob skills in the list, by key. Set this or abilities, not both." };
    glz::schema abilities{ .description = "Job ability IDs of a jug pet's Ready moves. Set this or skills, not both." };
};

template <>
struct glz::json_schema<xi::data::datasets::mob_skills::wire::Document>
{
    glz::schema mob_skills{ .description = "Mob skills keyed by their canonical name, in ID order." };
    glz::schema mob_skill_lists{ .description = "Skill lists keyed by name, in ID order." };
};
