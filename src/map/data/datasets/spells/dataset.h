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

#include <array>
#include <chrono>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace xi::data
{

struct BlueMagicData
{
    std::vector<uint16>                    MonsterSkills{};
    uint8                                  SetPoints{};
    std::string                            Trait{}; // key in data/blue_traits.yaml, empty for none
    uint8                                  TraitWeight{};
    std::vector<xi::SkillchainType>        Skillchain{};
    uint8                                  Knockback{};
    std::vector<std::pair<xi::Mod, int16>> Mods{};
};

struct SpellData
{
    uint16                          Id{};
    std::string                     Name{};
    xi::SpellGroup                  Group{};
    xi::SpellFamily                 Family{};
    xi::Element                     Element{};
    xi::SkillType                   Skill{};
    std::array<uint8, 24>           JobLevels{}; // indexed by xi::Job, 0 when the job cannot cast it
    xi::SpellRequirement            Requirements{};
    xi::Content                     Content{};
    uint16                          MpCost{};
    std::chrono::milliseconds       CastTime{};
    std::chrono::milliseconds       RecastTime{};
    xi::TargetType                  Targets{};
    float                           Range{};
    xi::SpellAoe                    Aoe{};
    float                           Radius{};
    xi::ZoneMisc                    ZoneMisc{};
    uint16                          Message{};
    uint16                          MagicBurstMessage{};
    uint16                          Animation{};
    std::chrono::milliseconds       AnimationTime{};
    uint16                          Base{};
    float                           Multiplier{};
    uint32                          CumulativeEnmity{};
    uint32                          VolatileEnmity{};
    std::optional<xi::StatusEffect> StatusEffect{};
    uint8                           StatusEffectTier{};
    std::optional<BlueMagicData>    BlueMagic{};
};

using Spells = std::vector<SpellData>; // ordered by id

} // namespace xi::data

namespace xi::data::datasets::spells::wire
{

struct Document;

}

namespace xi::data::datasets::spells
{

struct Dataset
{
    using Records      = Spells;
    using YamlDocument = wire::Document;

    static constexpr std::string_view kDataPath{ "spells" };
    static constexpr std::string_view kTitle{ "Spells" };
    static constexpr std::string_view kDescription{ "Spells, who can cast them, their costs and timings, and blue magic data." };

    static auto decode(std::string_view text) -> Records;
};

} // namespace xi::data::datasets::spells
