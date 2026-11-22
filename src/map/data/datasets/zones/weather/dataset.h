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

#include <map>
#include <string_view>

namespace xi::data
{

inline constexpr uint16 kWeatherCycleDays = 2160;

struct WeatherDayData
{
    xi::Weather Normal{};
    xi::Weather Common{};
    xi::Weather Rare{};
};

// Keyed by the day of the cycle each forecast takes effect; it holds until the next listed day.
using ZoneWeatherData = std::map<uint16, WeatherDayData>;

} // namespace xi::data

namespace xi::data::datasets::zones::weather::wire
{

struct Document;

}

namespace xi::data::datasets::zones::weather
{

struct Dataset
{
    using Records      = xi::data::ZoneWeatherData;
    using YamlDocument = wire::Document;

    static constexpr std::string_view kDataPath{ "weather" };
    static constexpr std::string_view kTitle{ "Zone weather" };
    static constexpr std::string_view kDescription{ "Weather forecast for each day of the 2160-day cycle, for one zone." };

    static auto decode(std::string_view text) -> Records;
};

} // namespace xi::data::datasets::zones::weather
