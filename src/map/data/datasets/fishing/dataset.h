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

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace xi::data
{

struct FishData
{
    std::string              Name;
    uint16                   Id{};
    bool                     Item{};
    xi::FishingSize          Size{};
    xi::FishingWater         Water{};
    uint8                    Skill{};
    uint8                    Difficulty{};
    uint8                    Delay{};
    uint8                    Move{};
    uint16                   MinLength{ 1 };
    uint16                   MaxLength{ 1 };
    uint16                   Ranking{};
    uint16                   Rarity{};
    uint8                    MaxHook{ 1 };
    xi::FishingFishFlag      Flags{};
    bool                     Legendary{};
    xi::FishingLegendaryFlag LegendaryFlags{};
    xi::KeyItem              KeyItem{};
    uint8                    Log{ 255 };
    uint8                    Quest{ 255 };
    bool                     QuestOnly{};
    bool                     Contest{};
    bool                     Disabled{};
};

struct FishingRodData
{
    std::string            Name;
    uint16                 Id{};
    xi::FishingRodMaterial Material{};
    xi::FishingSize        Size{};
    xi::FishingRodFlag     Flags{};
    uint16                 MinRank{};
    uint16                 MaxRank{};
    uint8                  Attack{};
    uint8                  Recovery{};
    uint8                  Time{};
    uint8                  SmallDelayBonus{};
    uint8                  SmallMoveBonus{};
    uint8                  LargeDelayBonus{};
    uint8                  LargeMoveBonus{};
    uint8                  Multiplier{};
    bool                   Legendary{};
    uint8                  LegendaryAttack{};
    uint8                  LegendaryTime{};
    bool                   Mmm{};
    std::string            BreaksInto; // item name, empty when the rod cannot break
};

struct FishingBaitData
{
    std::string                                          Name;
    uint16                                               Id{};
    xi::FishingBaitType                                  Type{};
    uint8                                                MaxHook{ 1 };
    xi::FishingBaitFlag                                  Flags{};
    bool                                                 Mmm{};
    std::vector<std::pair<uint16, xi::FishingBaitPower>> Catches; // fish id and power, ordered by fish id
};

struct FishingCatalog
{
    std::vector<FishData>        Fish;  // ordered by id
    std::vector<FishingRodData>  Rods;  // ordered by id
    std::vector<FishingBaitData> Baits; // ordered by id

    [[nodiscard]] auto size() const -> std::size_t
    {
        return Fish.size() + Rods.size() + Baits.size();
    }
};

} // namespace xi::data

namespace xi::data::datasets::fishing::wire
{

struct Document;

}

namespace xi::data::datasets::fishing
{

struct Dataset
{
    using Records      = FishingCatalog;
    using YamlDocument = wire::Document;

    static constexpr std::string_view kDataPath{ "fishing" };
    static constexpr std::string_view kTitle{ "Fishing" };
    static constexpr std::string_view kDescription{ "Fish and other catches, fishing rods, and baits with the fish they attract." };

    static auto decode(std::string_view text) -> Records;
};

} // namespace xi::data::datasets::fishing
