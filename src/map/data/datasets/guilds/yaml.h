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
#include "data/yaml/enum_keyed_map.h"
#include "data/yaml/schema_annotations.h"

#include <glaze/glaze.hpp>

#include <map>
#include <string>
#include <string_view>

namespace xi::data::datasets::guilds::wire
{

struct Pattern
{
    uint16                        max_points{};
    std::map<std::string, uint16> items;
};

struct Patterns
{
    Pattern a;
    Pattern b;
    Pattern c;
    Pattern d;
    Pattern e;
    Pattern f;
    Pattern g;
    Pattern h;
};

struct GuildEntry
{
    std::string                                 currency;
    yaml::EnumKeyedMap<xi::CraftRank, Patterns> ranks;
};

struct Document
{
    yaml::EnumKeyedMap<xi::Guild, GuildEntry> guilds;

    using YamlRoot = yaml::DatasetRoot<&Document::guilds>;
};

} // namespace xi::data::datasets::guilds::wire

template <>
struct glz::meta<xi::data::datasets::guilds::wire::Pattern>
{
    static constexpr std::string_view name{ "guild_point_pattern" };
};

template <>
struct glz::json_schema<xi::data::datasets::guilds::wire::Pattern>
{
    glz::schema max_points{ .description = "Most points a player can earn from this pattern in one day.", .minimum = 0L, .maximum = 65535L };
    glz::schema items{ .description = "Items the guild accepts, keyed by item name, with the points each one is worth." };
};

template <>
struct glz::meta<xi::data::datasets::guilds::wire::Patterns>
{
    static constexpr std::string_view name{ "guild_point_patterns" };
};

template <>
struct glz::json_schema<xi::data::datasets::guilds::wire::Patterns>
{
    glz::schema a{ .description = "Pattern A." };
    glz::schema b{ .description = "Pattern B." };
    glz::schema c{ .description = "Pattern C." };
    glz::schema d{ .description = "Pattern D." };
    glz::schema e{ .description = "Pattern E." };
    glz::schema f{ .description = "Pattern F." };
    glz::schema g{ .description = "Pattern G." };
    glz::schema h{ .description = "Pattern H." };
};

template <>
struct glz::meta<xi::data::datasets::guilds::wire::GuildEntry>
{
    static constexpr std::string_view name{ "guild" };
};

template <>
struct glz::json_schema<xi::data::datasets::guilds::wire::GuildEntry>
{
    glz::schema currency{ .description = "char_points column the guild's points are stored in." };
    glz::schema ranks{ .description = "Guild point items by the craft rank they are requested at, then by daily pattern." };
};

template <>
struct glz::json_schema<xi::data::datasets::guilds::wire::Document>
{
    glz::schema guilds{ .description = "Crafting guilds keyed by name." };
};
