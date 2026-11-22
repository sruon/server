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
#include "data/enums/mob_skill.h"
#include "data/enums/mob_skill_aoe.h"
#include "data/enums/mob_skill_flag.h"
#include "data/enums/skillchain_type.h"
#include "data/enums/target_type.h"

#include <chrono>
#include <string>
#include <string_view>
#include <vector>

namespace xi::data
{

struct MobSkillData
{
    xi::MobSkill                    Id{};
    std::string                     Name{}; // script name under scripts/actions/mobskills
    uint16                          Animation{};
    xi::MobSkillAoe                 Aoe{};
    float                           Radius{};
    float                           Distance{};
    std::chrono::milliseconds       AnimationTime{};
    std::chrono::milliseconds       ReadyTime{};
    xi::TargetType                  Target{};
    xi::MobSkillFlag                Flags{};
    int16                           Param{};
    uint8                           Knockback{};
    std::vector<xi::SkillchainType> Skillchain{};
};

struct MobSkillListData
{
    uint16              Id{};
    std::string         Name{};
    std::vector<uint16> Entries{}; // mob skill ids, or job ability ids for a jug pet's Ready list
};

struct MobSkills
{
    std::vector<MobSkillData>     Skills; // ordered by id
    std::vector<MobSkillListData> Lists;  // ordered by id

    auto size() const -> std::size_t
    {
        return Skills.size() + Lists.size();
    }
};

} // namespace xi::data

namespace xi::data::datasets::mob_skills::wire
{

struct Document;

}

namespace xi::data::datasets::mob_skills
{

struct Dataset
{
    using Records      = MobSkills;
    using YamlDocument = wire::Document;

    static constexpr std::string_view kDataPath{ "mob_skills" };
    static constexpr std::string_view kTitle{ "Mob Skills" };
    static constexpr std::string_view kDescription{ "Mob skills, their reach and timing, and the skill lists mobs and pets draw from." };

    static auto decode(std::string_view text) -> Records;
};

} // namespace xi::data::datasets::mob_skills
