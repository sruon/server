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
#include "data/enums/fishing_nm_flag.h"
#include "data/enums/zone.h"

#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace xi::data
{

enum class FishingAreaShape : uint8
{
    Zone     = 0,
    Cylinder = 1,
    Poly     = 2,
};

struct FishingAreaData
{
    std::string                                 Name;
    uint8                                       Id{};
    FishingAreaShape                            Shape{};
    std::array<float, 3>                        Center{}; // cylinder center; a poly only uses y, its floor
    uint16                                      Radius{};
    uint8                                       Height{};
    std::vector<std::array<float, 2>>           Corners; // x, z
    std::vector<std::pair<std::string, uint16>> Pool;    // fish name and rarity, ordered by name
};

struct FishingMonsterData
{
    std::string         Name;
    std::vector<uint32> Spawns;
    uint8               Area{}; // 0 for every area of the zone
    uint8               Level{};
    uint8               Difficulty{};
    uint8               Delay{ 10 };
    uint8               Move{ 15 };
    uint16              Ranking{ 10 };
    uint16              Rarity{ 1000 };
    bool                Nm{};
    xi::FishingNmFlag   NmFlags{};
    uint16              MinRespawn{};
    uint16              MaxRespawn{};
    std::string         Bait;
    std::string         AlternativeBait;
    uint8               Log{ 255 };
    uint8               Quest{ 255 };
    bool                QuestOnly{};
    bool                Disabled{};
};

struct ZoneFishing
{
    uint8                           Difficulty{};
    std::vector<FishingAreaData>    Areas; // ordered by id
    std::vector<FishingMonsterData> Monsters;

    [[nodiscard]] auto size() const -> std::size_t
    {
        return Areas.size() + Monsters.size();
    }
};

} // namespace xi::data

namespace xi::data::datasets::zones::fishing_areas::wire
{

struct Document;

}

namespace xi::data::datasets::zones::fishing_areas
{

struct Dataset
{
    using Records      = xi::data::ZoneFishing;
    using YamlDocument = wire::Document;

    static constexpr std::string_view kDataPath{ "fishing_areas" };
    static constexpr std::string_view kTitle{ "Zone fishing" };
    static constexpr std::string_view kDescription{ "Fishing areas, what bites in each, and monsters that can be fished up, for one zone." };

    static auto decode(std::string_view text) -> Records;
    static void verifyZone(const Records& records, xi::ZoneId zoneId);
};

} // namespace xi::data::datasets::zones::fishing_areas
