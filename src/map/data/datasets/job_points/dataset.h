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
#include "data/enums/job.h"
#include "data/enums/job_point.h"
#include "data/enums/mod.h"

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace xi::data
{

struct JobPointGiftData
{
    uint16  JobPointsSpent{};
    xi::Mod Mod{};
    int16   Value{};
};

struct JobPointGiftSpellData
{
    uint16      JobPointsSpent{};
    std::string Spell{}; // spell_list name
};

struct JobPointJobData
{
    std::vector<xi::JobPoint>          Categories{}; // ordered by id
    std::vector<JobPointGiftData>      Gifts{};      // ordered by job points spent
    std::vector<JobPointGiftSpellData> GiftSpells{}; // ordered by job points spent
};

using JobPoints = std::map<xi::Job, JobPointJobData>;

} // namespace xi::data

namespace xi::data::datasets::job_points::wire
{

struct Document;

}

namespace xi::data::datasets::job_points
{

struct Dataset
{
    using Records      = JobPoints;
    using YamlDocument = wire::Document;

    static constexpr std::string_view kDataPath{ "job_points" };
    static constexpr std::string_view kTitle{ "Job Points" };
    static constexpr std::string_view kDescription{ "Job point categories, gifts and gift spells by job." };

    static auto decode(std::string_view text) -> Records;
};

} // namespace xi::data::datasets::job_points
