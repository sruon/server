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
#include "data/enums/battlefield.h"
#include "data/enums/zone.h"

#include <chrono>
#include <utility>

#include <string>
#include <string_view>
#include <vector>

namespace xi::data
{

constexpr uint8 kBattlefieldIndexBits = 5;

// a battlefield is its zone plus its entry menu index
constexpr auto battlefieldId(const xi::ZoneId zone, const uint8 index) -> xi::BattlefieldId
{
    return static_cast<xi::BattlefieldId>((std::to_underlying(zone) << kBattlefieldIndexBits) | index);
}

struct BattlefieldData
{
    std::string          Name{};
    uint8                Index{};
    std::chrono::minutes TimeLimit{ 30 };
    uint8                MaxPlayers{ 6 };
    uint8                LevelCap{};      // 0 is no cap
    bool                 CapAtMaxLevel{}; // cap at the server's maximum level instead
    bool                 CanLoseExp{ true };
    bool                 AllowTrusts{};
    bool                 IsMission{};
    bool                 AllowSubjob{ true };
    bool                 CleanupDebuffs{};
    std::chrono::seconds DelayToExit{ 5 };
    bool                 HasWipeGrace{ true };
    bool                 ShowTimer{ true };
};

using Battlefields = std::vector<BattlefieldData>; // ordered by index

} // namespace xi::data

namespace xi::data::datasets::zones::battlefields::wire
{

struct Document;

}

namespace xi::data::datasets::zones::battlefields
{

struct Dataset
{
    using Records      = Battlefields;
    using YamlDocument = wire::Document;

    static constexpr std::string_view kDataPath{ "battlefields" };
    static constexpr std::string_view kTitle{ "Zone battlefields" };
    static constexpr std::string_view kDescription{ "Battlefields one zone hosts." };

    static auto decode(std::string_view text) -> Records;
    static void verifyZone(const Records& records, xi::ZoneId zoneId);
};

} // namespace xi::data::datasets::zones::battlefields
