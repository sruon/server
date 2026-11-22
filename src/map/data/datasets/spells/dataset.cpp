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

#include "data/datasets/spells/dataset.h"

#include "data/datasets/spells/yaml.h"
#include "data/yaml/read.h"

#include <fmt/format.h>

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace xi::data::datasets::spells
{

namespace
{

auto toMilliseconds(const std::optional<double>& seconds, const double fallback) -> std::chrono::milliseconds
{
    return std::chrono::milliseconds{ std::llround(seconds.value_or(fallback) * 1000.0) };
}

auto toBlueMagic(const wire::BlueMagic& source, const std::string_view name) -> BlueMagicData
{
    if (source.monster_skills.empty())
    {
        throw std::runtime_error(fmt::format("{} blue_magic needs at least one monster skill", name));
    }

    BlueMagicData blue{
        .MonsterSkills = source.monster_skills,
        .SetPoints     = source.set_points,
        .Trait         = source.trait.value_or(""),
        .Knockback     = source.knockback.value_or(0),
    };

    if (source.trait)
    {
        blue.TraitWeight = source.trait_weight.value_or(1);
    }
    else if (source.trait_weight)
    {
        throw std::runtime_error(fmt::format("{} sets trait_weight without a trait", name));
    }

    for (const auto& skillchain : source.skillchain.value_or(std::vector<yaml::EnumToken<xi::SkillchainType>>{}))
    {
        blue.Skillchain.emplace_back(yaml::resolveEnum(skillchain));
    }

    if (blue.Skillchain.size() > 3)
    {
        throw std::runtime_error(fmt::format("{} has more than three skillchain properties", name));
    }

    if (blue.Knockback > 7)
    {
        throw std::runtime_error(fmt::format("{} knockback {} is above 7", name, blue.Knockback));
    }

    for (const auto& [mod, value] : yaml::resolveKeys(source.mods))
    {
        blue.Mods.emplace_back(mod, value);
    }

    std::ranges::sort(blue.Mods);
    return blue;
}

} // namespace

auto Dataset::decode(const std::string_view text) -> Records
{
    const auto document = yaml::read<YamlDocument>(text);

    Records                    records;
    std::unordered_set<uint16> ids;

    for (const auto& [name, source] : document.spells)
    {
        if (source.id == 0 || source.id >= 1024)
        {
            throw std::runtime_error(fmt::format("{} has spell id {} outside 1-1023", name, source.id));
        }

        if (!ids.insert(source.id).second)
        {
            throw std::runtime_error(fmt::format("duplicate spell id {}", source.id));
        }

        SpellData record{
            .Id                = source.id,
            .Name              = name,
            .Group             = yaml::resolveEnum(source.group),
            .Family            = yaml::resolveEnum(source.family),
            .Element           = yaml::resolveEnum(source.element),
            .Skill             = yaml::resolveEnum(source.skill),
            .Requirements      = yaml::resolveFlags(source.requirements),
            .Content           = yaml::resolveEnum(source.content),
            .MpCost            = source.mp_cost.value_or(0),
            .CastTime          = toMilliseconds(source.cast_time, 0.0),
            .RecastTime        = toMilliseconds(source.recast_time, 0.0),
            .Targets           = yaml::resolveFlags(source.targets),
            .Range             = source.range.value_or(0.0f),
            .Aoe               = yaml::resolveEnum(source.aoe),
            .Radius            = source.radius.value_or(0.0f),
            .ZoneMisc          = yaml::resolveFlags(source.zone_misc),
            .Message           = source.message.value_or(0),
            .MagicBurstMessage = source.magic_burst_message.value_or(0),
            .Animation         = source.animation,
            .AnimationTime     = toMilliseconds(source.animation_time, 2.0),
            .Base              = source.base.value_or(0),
            .Multiplier        = source.multiplier.value_or(1.0f),
            .StatusEffectTier  = source.status_effect_tier.value_or(0),
        };

        if (record.Group == xi::SpellGroup::None)
        {
            throw std::runtime_error(fmt::format("{} needs a group", name));
        }

        for (const auto& [job, level] : yaml::resolveKeys(source.jobs))
        {
            if (job == xi::Job::NONE)
            {
                throw std::runtime_error(fmt::format("{} lists job none", name));
            }

            if (level == 0)
            {
                throw std::runtime_error(fmt::format("{} lists a job at level 0", name));
            }

            record.JobLevels[static_cast<uint8>(job)] = level;
        }

        if (source.enmity)
        {
            record.CumulativeEnmity = source.enmity->cumulative.value_or(0);
            record.VolatileEnmity   = source.enmity->volatile_.value_or(0);
        }

        if (source.status_effect)
        {
            record.StatusEffect = yaml::resolveEnum(*source.status_effect);
        }

        if (source.blue_magic)
        {
            if (record.Group != xi::SpellGroup::Blue)
            {
                throw std::runtime_error(fmt::format("{} has blue_magic but is not a blue spell", name));
            }

            record.BlueMagic = toBlueMagic(*source.blue_magic, name);
        }

        records.emplace_back(std::move(record));
    }

    std::ranges::sort(records, {}, &SpellData::Id);
    return records;
}

} // namespace xi::data::datasets::spells
