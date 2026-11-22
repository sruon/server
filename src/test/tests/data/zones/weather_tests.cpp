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

#include "map/data/datasets/zones/weather/dataset.h"
#include "map/data/loader.h"
#include "map/data/yaml/merge.h"
#include "map/weather_container.h"

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string>
#include <vector>

namespace
{

using WeatherDataset = xi::data::datasets::zones::weather::Dataset;

constexpr auto kCore = R"(
weather:
  0:    {normal: sunshine}
  5:    {normal: clouds, common: sunshine}
  9:    {normal: snow, common: clouds, rare: blizzards}
  12:   {normal: rain, rare: squall}
)";

} // namespace

TEST_CASE("weather: omitted tiers fall back to the one before", "[data][weather]")
{
    const auto records = WeatherDataset::decode(kCore);

    REQUIRE(records.size() == 4);

    CHECK(records.at(0).Normal == xi::Weather::Sunshine);
    CHECK(records.at(0).Common == xi::Weather::Sunshine);
    CHECK(records.at(0).Rare == xi::Weather::Sunshine);

    CHECK(records.at(5).Normal == xi::Weather::Clouds);
    CHECK(records.at(5).Common == xi::Weather::Sunshine);
    CHECK(records.at(5).Rare == xi::Weather::Sunshine);

    CHECK(records.at(9).Normal == xi::Weather::Snow);
    CHECK(records.at(9).Common == xi::Weather::Clouds);
    CHECK(records.at(9).Rare == xi::Weather::Blizzards);

    CHECK(records.at(12).Common == xi::Weather::Rain);
    CHECK(records.at(12).Rare == xi::Weather::Squall);
}

TEST_CASE("weather: a day past the cycle is rejected", "[data][weather]")
{
    constexpr auto pastCycle = R"(
weather:
  2160: {normal: sunshine}
)";

    REQUIRE_THROWS_AS(WeatherDataset::decode(pastCycle), std::runtime_error);
}

TEST_CASE("weather: an unknown weather name is rejected", "[data][weather]")
{
    constexpr auto unknown = R"(
weather:
  0:    {normal: drizzle}
)";

    REQUIRE_THROWS(WeatherDataset::decode(unknown));
}

TEST_CASE("weather: a module patch replaces, adds and removes days", "[data][weather][binding]")
{
    const std::vector<std::string> modules{ R"(
weather:
  5:    {normal: fog}
  9:    null
  100:  {normal: thunder, common: thunderstorms}
)" };

    const auto records = WeatherDataset::decode(xi::data::mergeYaml(kCore, modules));

    REQUIRE(records.size() == 4);
    CHECK(records.at(5).Normal == xi::Weather::Fog);
    CHECK(records.at(5).Common == xi::Weather::Sunshine);
    CHECK_FALSE(records.contains(9));
    CHECK(records.at(12).Rare == xi::Weather::Squall);
    CHECK(records.at(100).Normal == xi::Weather::Thunder);
    CHECK(records.at(100).Rare == xi::Weather::Thunderstorms);
}

TEST_CASE("weather: a forecast holds until the next listed day", "[data][weather]")
{
    const auto records = xi::data::loadZoneFile<WeatherDataset>(xi::ZoneId::WestRonfaure);
    REQUIRE(records.has_value());

    WeatherContainer weather;
    weather.load(*records);

    CHECK_FALSE(weather.isStatic());

    const auto carried = weather.entryForDay(1);
    CHECK(carried.normal == xi::Weather::Sunshine);
    CHECK(carried.common == xi::Weather::Clouds);
    CHECK(carried.rare == xi::Weather::Clouds);

    const auto changed = weather.entryForDay(2);
    CHECK(changed.normal == xi::Weather::Sunshine);
    CHECK(changed.common == xi::Weather::Sunshine);

    const auto last = weather.entryForDay(xi::data::kWeatherCycleDays - 1);
    CHECK(last.normal != xi::Weather::None);
}

TEST_CASE("weather: a zone with one forecast never cycles", "[data][weather]")
{
    const auto records = xi::data::loadZoneFile<WeatherDataset>(xi::ZoneId::Temenos);
    REQUIRE(records.has_value());

    WeatherContainer weather;
    weather.load(*records);

    CHECK(weather.isStatic());
    CHECK(weather.entryForDay(0).common == xi::Weather::StellarGlare);
    CHECK(weather.entryForDay(xi::data::kWeatherCycleDays - 1).common == xi::Weather::StellarGlare);
}
