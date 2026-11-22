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
#include "data/enums/craft_rank.h"
#include "data/enums/guild.h"

#include <array>
#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace xi::data
{

inline constexpr std::size_t kGuildPointPatterns = 8;

struct GuildPointItem
{
    std::string Item{};
    uint16      Points{};
};

struct GuildPointPattern
{
    uint16                      MaxPoints{};
    std::vector<GuildPointItem> Items{};
};

using GuildPointPatterns = std::array<GuildPointPattern, kGuildPointPatterns>;

struct GuildData
{
    xi::Guild                                   Id{};
    std::string                                 Currency{};
    std::map<xi::CraftRank, GuildPointPatterns> Ranks{};
};

using Guilds = std::vector<GuildData>; // ordered by id

} // namespace xi::data

namespace xi::data::datasets::guilds::wire
{

struct Document;

}

namespace xi::data::datasets::guilds
{

struct Dataset
{
    using Records      = Guilds;
    using YamlDocument = wire::Document;

    static constexpr std::string_view kDataPath{ "guilds" };
    static constexpr std::string_view kTitle{ "Guilds" };
    static constexpr std::string_view kDescription{ "Crafting guilds, their point currency, and the items they request each day." };

    static auto decode(std::string_view text) -> Records;
};

} // namespace xi::data::datasets::guilds
