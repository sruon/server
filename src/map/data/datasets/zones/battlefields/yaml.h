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
#include "data/yaml/schema_annotations.h"

#include <glaze/glaze.hpp>

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace xi::data::datasets::zones::battlefields::wire
{

// `max`, the server's maximum level
struct LevelCapKeyword
{
    std::string Value;
};

using LevelCap = std::variant<uint8, LevelCapKeyword>;

struct Battlefield
{
    uint8                                   index{};
    std::optional<uint16>                   time_limit;
    std::optional<uint8>                    max_players;
    std::optional<LevelCap>                 level_cap;
    std::optional<bool>                     can_lose_exp;
    std::optional<bool>                     allow_trusts;
    std::optional<bool>                     is_mission;
    std::optional<bool>                     allow_subjob;
    std::optional<bool>                     cleanup_debuffs;
    std::optional<uint8>                    delay_to_exit;
    std::optional<bool>                     has_wipe_grace;
    std::optional<bool>                     show_timer;
    std::optional<std::vector<std::string>> entry_npcs;
    std::optional<std::vector<std::string>> exit_npcs;
    std::optional<uint8>                    area;
    std::optional<std::vector<uint8>>       allowed_areas;
    std::optional<uint32>                   grant_xp;
    std::optional<bool>                     grant_xp_lockout;
    std::optional<bool>                     experimental;
    std::optional<bool>                     rhapsody_trusts;
};

struct Document
{
    std::optional<uint8>                    max_areas;
    std::optional<std::vector<std::string>> entry_npcs;
    std::optional<std::vector<std::string>> exit_npcs;
    std::map<std::string, Battlefield>      battlefields;
};

} // namespace xi::data::datasets::zones::battlefields::wire

template <>
struct glz::meta<xi::data::datasets::zones::battlefields::wire::LevelCapKeyword>
{
    using T                                 = xi::data::datasets::zones::battlefields::wire::LevelCapKeyword;
    static constexpr std::string_view name  = "level_cap_keyword";
    static constexpr auto             value = &T::Value;
};

template <>
struct glz::json_schema<xi::data::datasets::zones::battlefields::wire::LevelCapKeyword>
{
    glz::schema Value{ .enumeration = std::vector<std::string_view>{ "max" } };
};

template <>
struct glz::json_schema<xi::data::datasets::zones::battlefields::wire::Battlefield>
{
    glz::schema index{ .description = "Position in the zone's entry menu, sent to the client in the entry and win events.", .minimum = 0L, .maximum = 31L };
    glz::schema time_limit{ .description = "Minutes the party has to win.", .defaultValue = 30, .minimum = 1L, .maximum = 65535L };
    glz::schema max_players{ .description = "Most players that can enter.", .defaultValue = 6, .minimum = 1L, .maximum = 255L };
    glz::schema level_cap{ .description = "Level players are capped to inside, or `max` for the server's maximum level. Omit for no cap." };
    glz::schema can_lose_exp{ .description = "Dying inside costs experience points.", .defaultValue = true };
    glz::schema allow_trusts{ .description = "Trusts can be called inside.", .defaultValue = false };
    glz::schema is_mission{ .description = "A mission fight, capped to the server's maximum level when mission level caps are off.", .defaultValue = false };
    glz::schema allow_subjob{ .description = "Support jobs work inside.", .defaultValue = true };
    glz::schema cleanup_debuffs{ .description = "Erasable debuffs are removed from players who win.", .defaultValue = false };
    glz::schema delay_to_exit{ .description = "Seconds between the win and the exit cutscene.", .defaultValue = 5, .minimum = 0L, .maximum = 255L };
    glz::schema has_wipe_grace{ .description = "A wiped party gets 3 minutes to recover before it is removed.", .defaultValue = true };
    glz::schema show_timer{ .description = "The remaining time shows in the client.", .defaultValue = true };
    glz::schema entry_npcs{ .description = "NPCs that open the entry menu for this battlefield, by name. Omit to use the zone's.", .uniqueItems = true };
    glz::schema exit_npcs{ .description = "NPCs that lead out of this battlefield's arenas, by name. Omit to use the zone's, `[]` for none.", .uniqueItems = true };
    glz::schema area{ .description = "The one arena this battlefield always uses. Omit to use any free arena.", .minimum = 1L, .maximum = 15L };
    glz::schema allowed_areas{ .description = "Arenas this battlefield may use, for when only some have their mobs placed. Omit for every arena.", .uniqueItems = true };
    glz::schema grant_xp{ .description = "Experience points given to each player on a win." };
    glz::schema grant_xp_lockout{ .description = "The experience is only given once per JST day.", .defaultValue = false };
    glz::schema experimental{ .description = "Marked experimental: enemies are stronger and players are warned on entry.", .defaultValue = false };
    glz::schema rhapsody_trusts{ .description = "A player holding Rhapsody in Umber may call trusts even though only one player can enter.", .defaultValue = false };
};

template <>
struct glz::json_schema<xi::data::datasets::zones::battlefields::wire::Document>
{
    glz::schema max_areas{ .description = "Arenas the zone has, which is also how many of its battlefields can run at once.", .defaultValue = 3, .minimum = 1L, .maximum = 15L };
    glz::schema entry_npcs{ .description = "NPCs that open the entry menu, by name, for every battlefield that does not name its own.", .uniqueItems = true };
    glz::schema exit_npcs{ .description = "NPCs that lead out of the arenas, by name, for every battlefield that does not name its own.", .uniqueItems = true };
    glz::schema battlefields{ .description = "Battlefields the zone hosts, keyed by their canonical name in index order." };
};
