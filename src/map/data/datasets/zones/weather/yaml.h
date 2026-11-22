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
#include "data/enums/weather.h"
#include "data/yaml/enum_token.h"
#include "data/yaml/schema_annotations.h"

#include <glaze/glaze.hpp>

#include <map>
#include <optional>

namespace xi::data::datasets::zones::weather::wire
{

struct Day
{
    yaml::EnumToken<xi::Weather>                normal;
    std::optional<yaml::EnumToken<xi::Weather>> common;
    std::optional<yaml::EnumToken<xi::Weather>> rare;
};

struct Document
{
    std::map<uint16, Day> weather;

    using YamlRoot = yaml::DatasetRoot<&Document::weather>;
};

} // namespace xi::data::datasets::zones::weather::wire

template <>
struct glz::json_schema<xi::data::datasets::zones::weather::wire::Day>
{
    glz::schema normal{ .description = "Weather rolled half the time." };
    glz::schema common{ .description = "Weather rolled 35% of the time. Defaults to normal." };
    glz::schema rare{ .description = "Weather rolled 15% of the time. Defaults to common." };
};

template <>
struct glz::json_schema<xi::data::datasets::zones::weather::wire::Document>
{
    glz::schema weather{ .description = "Forecasts keyed by the day of the 2160-day cycle they take effect, from 0. Each holds until the next listed day. A zone with a single forecast never changes weather." };
};
