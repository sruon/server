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

#include "data/datasets/zones/fishing_areas/dataset.h"

#include "data/datasets/zones/fishing_areas/yaml.h"
#include "data/yaml/read.h"

#include <fmt/format.h>

#include <algorithm>
#include <map>
#include <ranges>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace xi::data::datasets::zones::fishing_areas
{

namespace
{

auto convertArea(const std::string& name, const wire::Area& source) -> FishingAreaData
{
    if (source.cylinder && source.poly)
    {
        throw std::runtime_error(fmt::format("area {} is a cylinder or a poly, not both", name));
    }

    FishingAreaData area{ .Name = name, .Id = source.id };

    if (source.cylinder)
    {
        if (source.cylinder->center.size() != 3)
        {
            throw std::runtime_error(fmt::format("area {} center must be x, y, z", name));
        }

        area.Shape  = FishingAreaShape::Cylinder;
        area.Center = { source.cylinder->center[0], source.cylinder->center[1], source.cylinder->center[2] };
        area.Radius = source.cylinder->radius;
        area.Height = source.cylinder->height;
    }

    if (source.poly)
    {
        if (source.poly->corners.size() < 3)
        {
            throw std::runtime_error(fmt::format("area {} needs at least three corners", name));
        }

        area.Shape  = FishingAreaShape::Poly;
        area.Center = { 0.0f, source.poly->floor, 0.0f };
        area.Height = source.poly->height;
        for (const auto& corner : source.poly->corners)
        {
            if (corner.size() != 2)
            {
                throw std::runtime_error(fmt::format("area {} corners must be x, z", name));
            }

            area.Corners.push_back({ corner[0], corner[1] });
        }
    }

    for (const auto& [fish, rarity] : source.pool.value_or(std::map<std::string, uint16>{}))
    {
        area.Pool.emplace_back(fish, rarity);
    }

    return area;
}

} // namespace

auto Dataset::decode(const std::string_view text) -> Records
{
    const auto document = yaml::read<YamlDocument>(text);

    Records                      records{ .Difficulty = document.difficulty.value_or(0) };
    std::map<std::string, uint8> areaIds;

    for (const auto& [name, source] : document.areas.value_or(std::map<std::string, wire::Area>{}))
    {
        if (source.id == 0)
        {
            throw std::runtime_error(fmt::format("area {} needs an id above 0", name));
        }

        if (std::ranges::any_of(areaIds | std::views::values, [&](const uint8 id)
                                {
                                    return id == source.id;
                                }))
        {
            throw std::runtime_error(fmt::format("duplicate area id {}", source.id));
        }

        areaIds.emplace(name, source.id);
        records.Areas.emplace_back(convertArea(name, source));
    }

    std::ranges::sort(records.Areas, {}, &FishingAreaData::Id);

    std::unordered_set<uint32> spawns;
    for (const auto& [name, source] : document.monsters.value_or(std::map<std::string, wire::Monster>{}))
    {
        FishingMonsterData monster{
            .Name            = name,
            .Spawns          = source.spawns,
            .Level           = source.level,
            .Difficulty      = source.difficulty,
            .Delay           = source.delay.value_or(10),
            .Move            = source.move.value_or(15),
            .Ranking         = source.ranking.value_or(10),
            .Rarity          = source.rarity.value_or(1000),
            .Nm              = source.nm.value_or(false),
            .NmFlags         = yaml::resolveFlags(source.nm_flags),
            .Bait            = source.bait.value_or(""),
            .AlternativeBait = source.alternative_bait.value_or(""),
            .Log             = source.log.value_or(255),
            .Quest           = source.quest.value_or(255),
            .QuestOnly       = source.quest_only.value_or(false),
            .Disabled        = source.disabled.value_or(false),
        };

        if (monster.Spawns.empty())
        {
            throw std::runtime_error(fmt::format("monster {} lists no spawns", name));
        }

        for (const auto spawn : monster.Spawns)
        {
            if (!spawns.insert(spawn).second)
            {
                throw std::runtime_error(fmt::format("spawn {} is listed twice", spawn));
            }
        }

        if (source.area)
        {
            const auto areaIt = areaIds.find(*source.area);
            if (areaIt == areaIds.end())
            {
                throw std::runtime_error(fmt::format("monster {} names unknown area '{}'", name, *source.area));
            }

            monster.Area = areaIt->second;
        }

        if (source.respawn)
        {
            if (source.respawn->size() != 2 || (*source.respawn)[0] > (*source.respawn)[1])
            {
                throw std::runtime_error(fmt::format("monster {} respawn must be [min, max]", name));
            }

            monster.MinRespawn = (*source.respawn)[0];
            monster.MaxRespawn = (*source.respawn)[1];
        }

        records.Monsters.emplace_back(std::move(monster));
    }

    return records;
}

void Dataset::verifyZone(const Records& records, const xi::ZoneId zoneId)
{
    for (const auto& monster : records.Monsters)
    {
        for (const auto spawn : monster.Spawns)
        {
            if (((spawn >> 12) & 0xFFF) != static_cast<uint32>(zoneId))
            {
                throw std::runtime_error(fmt::format("monster {} spawn {} is not in this zone", monster.Name, spawn));
            }
        }
    }
}

} // namespace xi::data::datasets::zones::fishing_areas
