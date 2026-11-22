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
#include "data/enums/fishing_bait_flag.h"
#include "data/enums/fishing_bait_power.h"
#include "data/enums/fishing_bait_type.h"
#include "data/enums/fishing_fish_flag.h"
#include "data/enums/fishing_legendary_flag.h"
#include "data/enums/fishing_rod_flag.h"
#include "data/enums/fishing_rod_material.h"
#include "data/enums/fishing_size.h"
#include "data/enums/fishing_water.h"
#include "data/enums/key_item.h"
#include "data/yaml/enum_token.h"
#include "data/yaml/schema_annotations.h"

#include <glaze/glaze.hpp>

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace xi::data::datasets::fishing::wire
{

struct Fish
{
    uint16                                                                id{};
    std::optional<bool>                                                   item;
    std::optional<yaml::EnumToken<xi::FishingSize>>                       size;
    std::optional<yaml::EnumToken<xi::FishingWater>>                      water;
    uint8                                                                 skill{};
    uint8                                                                 difficulty{};
    uint8                                                                 delay{};
    uint8                                                                 move{};
    std::optional<std::vector<uint16>>                                    length;
    uint16                                                                ranking{};
    uint16                                                                rarity{};
    std::optional<uint8>                                                  max_hook;
    std::optional<std::vector<yaml::EnumToken<xi::FishingFishFlag>>>      flags;
    std::optional<bool>                                                   legendary;
    std::optional<std::vector<yaml::EnumToken<xi::FishingLegendaryFlag>>> legendary_flags;
    std::optional<yaml::EnumToken<xi::KeyItem>>                           key_item;
    std::optional<uint8>                                                  log;
    std::optional<uint8>                                                  quest;
    std::optional<bool>                                                   quest_only;
    std::optional<bool>                                                   contest;
    std::optional<bool>                                                   disabled;
};

struct Bonus
{
    std::optional<uint8> delay;
    std::optional<uint8> move;
};

struct Rod
{
    uint16                                                          id{};
    std::optional<yaml::EnumToken<xi::FishingRodMaterial>>          material;
    std::optional<yaml::EnumToken<xi::FishingSize>>                 size;
    std::optional<std::vector<yaml::EnumToken<xi::FishingRodFlag>>> flags;
    std::vector<uint16>                                             rank;
    uint8                                                           attack{};
    uint8                                                           recovery{};
    uint8                                                           time{};
    std::optional<Bonus>                                            small_bonus;
    std::optional<Bonus>                                            large_bonus;
    std::optional<uint8>                                            multiplier;
    std::optional<bool>                                             legendary;
    std::optional<uint8>                                            legendary_attack;
    std::optional<uint8>                                            legendary_time;
    std::optional<bool>                                             mmm;
    std::optional<std::string>                                      breaks_into;
};

struct Bait
{
    uint16                                                                      id{};
    std::optional<yaml::EnumToken<xi::FishingBaitType>>                         type;
    std::optional<uint8>                                                        max_hook;
    std::optional<std::vector<yaml::EnumToken<xi::FishingBaitFlag>>>            flags;
    std::optional<bool>                                                         mmm;
    std::optional<std::map<std::string, yaml::EnumToken<xi::FishingBaitPower>>> catches;
};

struct Fishing
{
    std::map<std::string, Fish> fish;
    std::map<std::string, Rod>  rods;
    std::map<std::string, Bait> baits;
};

struct Document
{
    Fishing fishing;

    using YamlRoot = yaml::DatasetRoot<&Document::fishing>;
};

} // namespace xi::data::datasets::fishing::wire

template <>
struct glz::json_schema<xi::data::datasets::fishing::wire::Fish>
{
    glz::schema id{ .description = "Item ID of the catch.", .minimum = 1L, .maximum = 65535L };
    glz::schema item{ .description = "An item rather than a fish. Items bite without bait.", .defaultValue = false };
    glz::schema size{ .description = "Size class. Omitted means small." };
    glz::schema water{ .description = "Water it lives in. Omitted means any." };
    glz::schema skill{ .description = "Fishing skill level it is rated at.", .minimum = 0L, .maximum = 255L };
    glz::schema difficulty{ .description = "Fight difficulty, used by most hook and rod rolls.", .minimum = 0L, .maximum = 255L };
    glz::schema delay{ .description = "Base arrow delay while fighting it.", .minimum = 0L, .maximum = 255L };
    glz::schema move{ .description = "Base movement while fighting it.", .minimum = 0L, .maximum = 255L };
    glz::schema length{ .description = "Minimum and maximum length, in ilms. Omitted means [1, 1].", .minItems = 2UL, .maxItems = 2UL };
    glz::schema ranking{ .description = "Ranking against the rod's rank. 99 or more keeps it out of the game.", .minimum = 0L, .maximum = 255L };
    glz::schema rarity{ .description = "Hook weight from 0 to 1000; higher is more common.", .minimum = 0L, .maximum = 1000L };
    glz::schema max_hook{ .description = "How many can be hooked at once.", .defaultValue = 1, .minimum = 1L, .maximum = 255L };
    glz::schema flags{ .description = "Special handling.", .uniqueItems = true };
    glz::schema legendary{ .description = "Legendary fish.", .defaultValue = false };
    glz::schema legendary_flags{ .description = "Fight time rules for a legendary fish.", .uniqueItems = true };
    glz::schema key_item{ .description = "Key item needed to hook it." };
    glz::schema log{ .description = "Quest log the quest field belongs to.", .defaultValue = 255, .minimum = 0L, .maximum = 255L };
    glz::schema quest{ .description = "Quest the catch belongs to.", .defaultValue = 255, .minimum = 0L, .maximum = 255L };
    glz::schema quest_only{ .description = "Only bites while its quest asks for it.", .defaultValue = false };
    glz::schema contest{ .description = "Can be the fish of a fishing contest.", .defaultValue = false };
    glz::schema disabled{ .description = "Kept in the file but left out of the game.", .defaultValue = false };
};

template <>
struct glz::json_schema<xi::data::datasets::fishing::wire::Bonus>
{
    glz::schema delay{ .description = "Arrow delay bonus.", .defaultValue = 0, .minimum = 0L, .maximum = 255L };
    glz::schema move{ .description = "Movement bonus.", .defaultValue = 0, .minimum = 0L, .maximum = 255L };
};

template <>
struct glz::json_schema<xi::data::datasets::fishing::wire::Rod>
{
    glz::schema id{ .description = "Item ID of the rod.", .minimum = 1L, .maximum = 65535L };
    glz::schema material{ .description = "What the rod is made of. Omitted means wood." };
    glz::schema size{ .description = "Size class of fish the rod is made for. Omitted means small." };
    glz::schema flags{ .description = "Special handling.", .uniqueItems = true };
    glz::schema rank{ .description = "Minimum and maximum rank.", .minItems = 2UL, .maxItems = 2UL };
    glz::schema attack{ .description = "Damage multiplier against the fish.", .minimum = 0L, .maximum = 255L };
    glz::schema recovery{ .description = "Fish recovery multiplier.", .minimum = 0L, .maximum = 255L };
    glz::schema time{ .description = "Base fight time limit.", .minimum = 0L, .maximum = 255L };
    glz::schema small_bonus{ .description = "Arrow delay and movement bonus on small fish." };
    glz::schema large_bonus{ .description = "Arrow delay and movement bonus on large fish." };
    glz::schema multiplier{ .description = "Multiplier used by the fight time formulas.", .defaultValue = 0, .minimum = 0L, .maximum = 255L };
    glz::schema legendary{ .description = "Legendary rod.", .defaultValue = false };
    glz::schema legendary_attack{ .description = "Attack added against legendary fish.", .defaultValue = 0, .minimum = 0L, .maximum = 255L };
    glz::schema legendary_time{ .description = "Time added against legendary fish.", .defaultValue = 0, .minimum = 0L, .maximum = 255L };
    glz::schema mmm{ .description = "Moblin Maze Mongers rod.", .defaultValue = false };
    glz::schema breaks_into{ .description = "Item the rod turns into when it breaks. Omit for a rod that cannot break." };
};

template <>
struct glz::json_schema<xi::data::datasets::fishing::wire::Bait>
{
    glz::schema id{ .description = "Item ID of the bait.", .minimum = 1L, .maximum = 65535L };
    glz::schema type{ .description = "Kind of bait. Omitted means bait." };
    glz::schema max_hook{ .description = "How many fish it can hook at once.", .defaultValue = 1, .minimum = 1L, .maximum = 255L };
    glz::schema flags{ .description = "Special handling.", .uniqueItems = true };
    glz::schema mmm{ .description = "Moblin Maze Mongers bait.", .defaultValue = false };
    glz::schema catches{ .description = "Fish the bait attracts, keyed by fish name, with how well." };
};

template <>
struct glz::json_schema<xi::data::datasets::fishing::wire::Fishing>
{
    glz::schema fish{ .description = "Fish and items that can be hooked, keyed by item name." };
    glz::schema rods{ .description = "Fishing rods, keyed by item name." };
    glz::schema baits{ .description = "Baits and lures, keyed by item name." };
};

template <>
struct glz::json_schema<xi::data::datasets::fishing::wire::Document>
{
    glz::schema fishing{ .description = "Fishing catalog." };
};
