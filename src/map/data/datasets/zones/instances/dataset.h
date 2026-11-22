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
#include "common/types/maybe.h"
#include "common/types/position.h"
#include "data/enums/zone.h"

#include <chrono>
#include <string>
#include <string_view>
#include <vector>

namespace xi::data
{

struct InstanceMusicData
{
    Maybe<uint16> Day;
    Maybe<uint16> Night;
    Maybe<uint16> BattleSolo;
    Maybe<uint16> BattleParty;
};

struct InstanceData
{
    std::string          Name;
    uint16               Id{};
    xi::ZoneId           Entrance{};
    uint16               Overlay{};
    std::chrono::minutes TimeLimit{};
    position_t           Entry{};
    InstanceMusicData    Music;
};

using Instances = std::vector<InstanceData>;

} // namespace xi::data

namespace xi::data::datasets::zones::instances::wire
{

struct Document;

}

namespace xi::data::datasets::zones::instances
{

struct Dataset
{
    using Records      = xi::data::Instances;
    using YamlDocument = wire::Document;

    static constexpr std::string_view kDataPath{ "instances" };
    static constexpr std::string_view kTitle{ "Zone Instances" };
    static constexpr std::string_view kDescription{ "Instances run in one instanced zone. Their entities are the spawns and NPCs tagged with the instance's name." };

    static auto decode(std::string_view text) -> Records;
};

} // namespace xi::data::datasets::zones::instances
