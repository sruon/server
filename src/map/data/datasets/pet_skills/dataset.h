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
#include "data/enums/pet_skill.h"
#include "data/enums/skill_flag.h"
#include "data/enums/skillchain_type.h"
#include "data/enums/target_type.h"
#include "data/shared_types/action_area/dataset.h"

#include <chrono>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace xi::data
{

using PetSkillArea = std::variant<AoeSphereData, AoeConeData>;

struct PetSkillData
{
    xi::PetSkill                    Id{};
    std::string                     Name{}; // also names the script under scripts/actions/abilities/pets
    uint16                          MobSkill{};
    uint16                          Animation{};
    float                           Range{};
    xi::TargetType                  Target{};
    std::optional<PetSkillArea>     Area{};
    float                           MasterRange{};
    std::chrono::milliseconds       AnimationTime{};
    std::chrono::milliseconds       PrepareTime{};
    uint16                          Message{};
    xi::SkillFlag                   Flags{};
    int16                           Param{};
    uint8                           FinishCategory{};
    uint8                           Knockback{};
    std::vector<xi::SkillchainType> Skillchain{};
};

using PetSkills = std::vector<PetSkillData>; // ordered by id

} // namespace xi::data

namespace xi::data::datasets::pet_skills::wire
{

struct Document;

}

namespace xi::data::datasets::pet_skills
{

struct Dataset
{
    using Records      = PetSkills;
    using YamlDocument = wire::Document;

    static constexpr std::string_view kDataPath{ "pet_skills" };
    static constexpr std::string_view kTitle{ "Pet Skills" };
    static constexpr std::string_view kDescription{ "Blood pacts, breaths, ready moves and luopan skills, and how they reach their targets." };

    static auto decode(std::string_view text) -> Records;
};

} // namespace xi::data::datasets::pet_skills
