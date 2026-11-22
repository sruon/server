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
#include "data/enums/job.h"
#include "data/enums/job_ability.h"
#include "data/enums/merit.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace xi::data
{

struct AbilityChargeTier
{
    uint8     Level{};
    uint8     MaxCharges{};
    uint32    ChargeTime{}; // seconds
    xi::Merit Merit{};
};

struct AbilityCharges
{
    uint16                         RecastId{};
    std::vector<AbilityChargeTier> Tiers{};
};

struct AbilityData
{
    xi::JobAbility                Id{};
    std::string                   Name{};
    xi::Job                       Job{};
    uint8                         Level{};
    xi::ActionTarget              Target{};
    float                         Range{};
    bool                          Aoe{};
    uint8                         Radius{}; // yalms
    xi::AbilityCategory           Category{};
    uint16                        Animation{};
    uint16                        AnimationTime{}; // milliseconds
    uint16                        CastTime{};      // milliseconds
    uint16                        Message{};
    uint16                        Recast{}; // seconds
    uint16                        RecastId{};
    int32                         CE{};
    int32                         VE{};
    xi::Merit                     Merit{};
    xi::AbilityAddType            AddType{};
    xi::Content                   Content{};
    std::optional<AbilityCharges> Charges{};
};

using Abilities = std::vector<AbilityData>; // ordered by id, enum-only entries left out

} // namespace xi::data

namespace xi::data::datasets::abilities::wire
{

struct Document;

}

namespace xi::data::datasets::abilities
{

struct Dataset
{
    using Records      = Abilities;
    using YamlDocument = wire::Document;

    static constexpr std::string_view kDataPath{ "abilities" };
    static constexpr std::string_view kTitle{ "Abilities" };
    static constexpr std::string_view kDescription{ "Job and pet abilities grouped by job: who learns them, their targets, recasts and charges." };

    static auto decode(std::string_view text) -> Records;
};

} // namespace xi::data::datasets::abilities
