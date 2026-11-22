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
#include "data/enums/content.h"
#include "data/enums/element.h"
#include "data/enums/job.h"
#include "data/enums/mod.h"
#include "data/enums/skill_type.h"
#include "data/enums/skillchain_type.h"
#include "data/enums/spell_aoe.h"
#include "data/enums/spell_family.h"
#include "data/enums/spell_group.h"
#include "data/enums/spell_requirement.h"
#include "data/enums/status_effect.h"
#include "data/enums/target_type.h"
#include "data/enums/zone_misc.h"
#include "data/yaml/enum_keyed_map.h"
#include "data/yaml/enum_token.h"
#include "data/yaml/schema_annotations.h"

#include <glaze/glaze.hpp>

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace xi::data::datasets::spells::wire
{

struct Enmity
{
    std::optional<uint32> cumulative;
    std::optional<uint32> volatile_;
};

struct BlueMagic
{
    std::vector<uint16>                                             monster_skills;
    uint8                                                           set_points{};
    std::optional<std::string>                                      trait;
    std::optional<uint8>                                            trait_weight;
    std::optional<std::vector<yaml::EnumToken<xi::SkillchainType>>> skillchain;
    std::optional<uint8>                                            knockback;
    std::optional<yaml::EnumKeyedMap<xi::Mod, int16>>               mods;
};

struct Spell
{
    uint16                                                            id{};
    yaml::EnumToken<xi::SpellGroup>                                   group;
    std::optional<yaml::EnumToken<xi::SpellFamily>>                   family;
    std::optional<yaml::EnumToken<xi::Element>>                       element;
    std::optional<yaml::EnumToken<xi::SkillType>>                     skill;
    std::optional<yaml::EnumKeyedMap<xi::Job, uint8>>                 jobs;
    std::optional<std::vector<yaml::EnumToken<xi::SpellRequirement>>> requirements;
    std::optional<yaml::EnumToken<xi::Content>>                       content;
    std::optional<uint16>                                             mp_cost;
    std::optional<double>                                             cast_time;
    std::optional<double>                                             recast_time;
    std::optional<std::vector<yaml::EnumToken<xi::TargetType>>>       targets;
    std::optional<float>                                              range;
    std::optional<yaml::EnumToken<xi::SpellAoe>>                      aoe;
    std::optional<float>                                              radius;
    std::optional<std::vector<yaml::EnumToken<xi::ZoneMisc>>>         zone_misc;
    std::optional<uint16>                                             message;
    std::optional<uint16>                                             magic_burst_message;
    uint16                                                            animation{};
    std::optional<double>                                             animation_time;
    std::optional<uint16>                                             base;
    std::optional<float>                                              multiplier;
    std::optional<Enmity>                                             enmity;
    std::optional<yaml::EnumToken<xi::StatusEffect>>                  status_effect;
    std::optional<uint8>                                              status_effect_tier;
    std::optional<BlueMagic>                                          blue_magic;
};

struct Document
{
    std::map<std::string, Spell> spells;

    using YamlRoot = yaml::DatasetRoot<&Document::spells>;
};

} // namespace xi::data::datasets::spells::wire

template <>
struct glz::meta<xi::data::datasets::spells::wire::Enmity>
{
    using T                     = xi::data::datasets::spells::wire::Enmity;
    static constexpr auto value = glz::object("cumulative", &T::cumulative, "volatile", &T::volatile_);
};

template <>
struct glz::json_schema<xi::data::datasets::spells::wire::BlueMagic>
{
    glz::schema monster_skills{ .description = "Monster skills that teach the spell.", .minItems = 1UL, .uniqueItems = true };
    glz::schema set_points{ .description = "Blue magic points the spell costs to set.", .minimum = 0L, .maximum = 255L };
    glz::schema trait{ .description = "Blue trait category, a key in data/blue_traits.yaml, the spell adds weight to." };
    glz::schema trait_weight{ .description = "Weight the spell adds to its trait category.", .defaultValue = 1, .minimum = 0L, .maximum = 255L };
    glz::schema skillchain{ .description = "Skillchain properties, highest priority first.", .maxItems = 3UL, .uniqueItems = true };
    glz::schema knockback{ .description = "Knockback level.", .defaultValue = 0, .minimum = 0L, .maximum = 7L };
    glz::schema mods{ .description = "Stat bonuses granted while the spell is set." };
};

template <>
struct glz::json_schema<xi::data::datasets::spells::wire::Spell>
{
    glz::schema id{ .description = "Client-facing spell ID.", .minimum = 1L, .maximum = 1023L };
    glz::schema group{ .description = "Magic type. Also picks the scripts/actions/spells directory the script loads from." };
    glz::schema family{ .description = "Spell family, used to compare tiers of the same spell." };
    glz::schema element{ .description = "Element of the spell." };
    glz::schema skill{ .description = "Magic skill the spell uses." };
    glz::schema jobs{ .description = "Level each job learns the spell at. Omit jobs that cannot cast it." };
    glz::schema requirements{ .description = "Extra conditions for casting it, beyond job and level.", .uniqueItems = true };
    glz::schema content{ .description = "Content tag gating the spell. Omitted means always enabled." };
    glz::schema mp_cost{ .description = "MP cost.", .defaultValue = 0, .minimum = 0L };
    glz::schema cast_time{ .description = "Casting time in seconds.", .defaultValue = 0, .minimum = 0L };
    glz::schema recast_time{ .description = "Recast time in seconds.", .defaultValue = 0, .minimum = 0L };
    glz::schema targets{ .description = "Who the spell can be cast on.", .uniqueItems = true };
    glz::schema range{ .description = "Casting range in yalms.", .defaultValue = 0, .minimum = 0L };
    glz::schema aoe{ .description = "Area behaviour." };
    glz::schema radius{ .description = "Area radius in yalms.", .defaultValue = 0, .minimum = 0L };
    glz::schema zone_misc{ .description = "Zone flags a zone must allow for the spell to be cast there.", .uniqueItems = true };
    glz::schema message{ .description = "Message ID sent when the spell lands.", .defaultValue = 0, .minimum = 0L };
    glz::schema magic_burst_message{ .description = "Message ID sent when the spell lands as a magic burst.", .defaultValue = 0, .minimum = 0L };
    glz::schema animation{ .description = "Animation ID sent in the action packet.", .minimum = 0L };
    glz::schema animation_time{ .description = "Animation lock in seconds.", .defaultValue = 2, .minimum = 0L };
    glz::schema base{ .description = "Base value scripts can read.", .defaultValue = 0, .minimum = 0L };
    glz::schema multiplier{ .description = "Multiplier scripts can read.", .defaultValue = 1, .minimum = 0L };
    glz::schema enmity{ .description = "Enmity the spell generates." };
    glz::schema status_effect{ .description = "Effect a mob checks on its target before casting this buff. Omit to always allow it." };
    glz::schema status_effect_tier{ .description = "Tier the spell stamps on that effect.", .defaultValue = 0, .minimum = 0L, .maximum = 255L };
    glz::schema blue_magic{ .description = "Blue magic data. Only for blue spells." };
};

template <>
struct glz::json_schema<xi::data::datasets::spells::wire::Document>
{
    glz::schema spells{ .description = "Spells keyed by their script name, in ID order." };
};
