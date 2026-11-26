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
#include "data/enums/mod.h"
#include "data/yaml/enum_keyed_map.h"
#include "data/yaml/schema_annotations.h"

#include <glaze/glaze.hpp>

#include <map>
#include <optional>
#include <string>

namespace xi::data::datasets::job_points::wire
{

struct Category
{
    uint16 id{};
};

using Gifts = std::map<uint16, yaml::EnumKeyedMap<xi::Mod, int16>>;

struct Job
{
    std::map<std::string, Category> categories;
    std::optional<Gifts>            gifts;
};

struct Document
{
    yaml::EnumKeyedMap<xi::Job, Job> job_points;

    using YamlRoot = yaml::DatasetRoot<&Document::job_points>;
};

} // namespace xi::data::datasets::job_points::wire

template <>
struct glz::json_schema<xi::data::datasets::job_points::wire::Category>
{
    glz::schema id{ .description = "Client-facing job point ID: job ID * 32 + slot 0-9.", .minimum = 32L, .maximum = 713L };
};

template <>
struct glz::json_schema<xi::data::datasets::job_points::wire::Job>
{
    glz::schema categories{ .description = "The job's ten job point categories keyed by their canonical name, in ID order." };
    glz::schema gifts{ .description = "Mods granted at level 99, keyed by job points spent." };
};

template <>
struct glz::json_schema<xi::data::datasets::job_points::wire::Document>
{
    glz::schema job_points{ .description = "Job point categories and gifts by job." };
};
