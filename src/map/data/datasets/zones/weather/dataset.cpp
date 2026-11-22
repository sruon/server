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

#include "data/datasets/zones/weather/dataset.h"

#include "data/datasets/zones/weather/yaml.h"
#include "data/yaml/read.h"

#include <fmt/format.h>

#include <stdexcept>
#include <string_view>

namespace xi::data::datasets::zones::weather
{

auto Dataset::decode(const std::string_view text) -> Records
{
    const auto document = yaml::read<YamlDocument>(text);

    Records records;
    for (const auto& [day, source] : document.weather)
    {
        if (day >= kWeatherCycleDays)
        {
            throw std::runtime_error(fmt::format("weather day {} is past the end of the {}-day cycle", day, kWeatherCycleDays));
        }

        const auto normal = yaml::resolveEnum(source.normal);
        const auto common = yaml::resolveEnum(source.common, normal);

        records.emplace(day, WeatherDayData{ .Normal = normal, .Common = common, .Rare = yaml::resolveEnum(source.rare, common) });
    }

    return records;
}

} // namespace xi::data::datasets::zones::weather
