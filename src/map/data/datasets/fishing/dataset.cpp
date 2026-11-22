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

#include "data/datasets/fishing/dataset.h"

#include "data/datasets/fishing/yaml.h"
#include "data/yaml/read.h"

#include <fmt/format.h>

#include <algorithm>
#include <map>
#include <stdexcept>
#include <tuple>
#include <unordered_set>
#include <utility>

namespace xi::data::datasets::fishing
{

namespace
{

auto toRange(const std::vector<uint16>& values, const std::string_view what) -> std::pair<uint16, uint16>
{
    if (values.size() != 2 || values[0] > values[1])
    {
        throw std::runtime_error(fmt::format("{} must be [min, max]", what));
    }

    return { values[0], values[1] };
}

void claimId(std::unordered_set<uint16>& ids, const uint16 id, const std::string_view section)
{
    if (!ids.insert(id).second)
    {
        throw std::runtime_error(fmt::format("duplicate {} id {}", section, id));
    }
}

} // namespace

auto Dataset::decode(const std::string_view text) -> Records
{
    const auto document = yaml::read<YamlDocument>(text);

    Records                       records;
    std::map<std::string, uint16> fishIds;
    std::unordered_set<uint16>    ids;

    for (const auto& [name, source] : document.fishing.fish)
    {
        claimId(ids, source.id, "fish");
        fishIds.emplace(name, source.id);

        FishData record{
            .Name           = name,
            .Id             = source.id,
            .Item           = source.item.value_or(false),
            .Size           = yaml::resolveEnum(source.size),
            .Water          = yaml::resolveEnum(source.water),
            .Skill          = source.skill,
            .Difficulty     = source.difficulty,
            .Delay          = source.delay,
            .Move           = source.move,
            .Ranking        = source.ranking,
            .Rarity         = source.rarity,
            .MaxHook        = source.max_hook.value_or(1),
            .Flags          = yaml::resolveFlags(source.flags),
            .Legendary      = source.legendary.value_or(false),
            .LegendaryFlags = yaml::resolveFlags(source.legendary_flags),
            .KeyItem        = yaml::resolveEnum(source.key_item),
            .Log            = source.log.value_or(255),
            .Quest          = source.quest.value_or(255),
            .QuestOnly      = source.quest_only.value_or(false),
            .Contest        = source.contest.value_or(false),
            .Disabled       = source.disabled.value_or(false),
        };

        if (source.length)
        {
            std::tie(record.MinLength, record.MaxLength) = toRange(*source.length, fmt::format("{} length", name));
        }

        records.Fish.emplace_back(std::move(record));
    }

    ids.clear();
    for (const auto& [name, source] : document.fishing.rods)
    {
        claimId(ids, source.id, "rod");

        const auto smallBonus = source.small_bonus.value_or(wire::Bonus{});
        const auto largeBonus = source.large_bonus.value_or(wire::Bonus{});

        FishingRodData record{
            .Name            = name,
            .Id              = source.id,
            .Material        = yaml::resolveEnum(source.material),
            .Size            = yaml::resolveEnum(source.size),
            .Flags           = yaml::resolveFlags(source.flags),
            .Attack          = source.attack,
            .Recovery        = source.recovery,
            .Time            = source.time,
            .SmallDelayBonus = smallBonus.delay.value_or(0),
            .SmallMoveBonus  = smallBonus.move.value_or(0),
            .LargeDelayBonus = largeBonus.delay.value_or(0),
            .LargeMoveBonus  = largeBonus.move.value_or(0),
            .Multiplier      = source.multiplier.value_or(0),
            .Legendary       = source.legendary.value_or(false),
            .LegendaryAttack = source.legendary_attack.value_or(0),
            .LegendaryTime   = source.legendary_time.value_or(0),
            .Mmm             = source.mmm.value_or(false),
            .BreaksInto      = source.breaks_into.value_or(""),
        };

        if (source.rank.size() != 2)
        {
            throw std::runtime_error(fmt::format("{} rank must be [min, max]", name));
        }

        record.MinRank = source.rank[0];
        record.MaxRank = source.rank[1];
        records.Rods.emplace_back(std::move(record));
    }

    ids.clear();
    for (const auto& [name, source] : document.fishing.baits)
    {
        claimId(ids, source.id, "bait");

        FishingBaitData record{
            .Name    = name,
            .Id      = source.id,
            .Type    = yaml::resolveEnum(source.type),
            .MaxHook = source.max_hook.value_or(1),
            .Flags   = yaml::resolveFlags(source.flags),
            .Mmm     = source.mmm.value_or(false),
        };

        for (const auto& [fish, power] : source.catches.value_or(std::map<std::string, yaml::EnumToken<xi::FishingBaitPower>>{}))
        {
            const auto fishIt = fishIds.find(fish);
            if (fishIt == fishIds.end())
            {
                throw std::runtime_error(fmt::format("bait {} catches unknown fish '{}'", name, fish));
            }

            record.Catches.emplace_back(fishIt->second, yaml::resolveEnum(power));
        }

        std::ranges::sort(record.Catches);
        records.Baits.emplace_back(std::move(record));
    }

    std::ranges::sort(records.Fish, {}, &FishData::Id);
    std::ranges::sort(records.Rods, {}, &FishingRodData::Id);
    std::ranges::sort(records.Baits, {}, &FishingBaitData::Id);
    return records;
}

} // namespace xi::data::datasets::fishing
