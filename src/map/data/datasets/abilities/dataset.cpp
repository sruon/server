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

#include "data/datasets/abilities/dataset.h"

#include "data/datasets/abilities/yaml.h"
#include "data/yaml/read.h"

#include <fmt/format.h>

#include <algorithm>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace xi::data::datasets::abilities
{

auto Dataset::decode(const std::string_view text) -> Records
{
    const auto document = yaml::read<YamlDocument>(text);

    Records                    records;
    std::unordered_set<uint16> ids;
    std::unordered_set<uint16> chargedRecasts;

    for (const auto& [jobName, group] : document.abilities)
    {
        const auto job = yaml::resolveEnum(yaml::EnumToken<xi::Job>{ jobName });
        for (const auto& [name, source] : group)
        {
            yaml::verifyNamedMapEntry<xi::JobAbility>(name, source.id);
            if (!ids.insert(source.id).second)
            {
                throw std::runtime_error(fmt::format("duplicate ability id {}", source.id));
            }

            if (source.enum_only.value_or(false))
            {
                continue;
            }

            if (!source.animation)
            {
                throw std::runtime_error(fmt::format("{} needs an animation", name));
            }

            AbilityData record{
                .Id            = static_cast<xi::JobAbility>(source.id),
                .Name          = name,
                .Job           = job,
                .Level         = source.level.value_or(0),
                .Target        = yaml::resolveFlags(source.target),
                .Range         = source.range.value_or(0.0f),
                .Aoe           = source.aoe.value_or(false),
                .Radius        = source.radius.value_or(0),
                .Category      = yaml::resolveEnum(source.category, xi::AbilityCategory::AbilityFinish),
                .Animation     = *source.animation,
                .AnimationTime = source.animation_time.value_or(2000),
                .CastTime      = source.cast_time.value_or(0),
                .Message       = source.message.value_or(0),
                .Recast        = source.recast.value_or(0),
                .RecastId      = source.recast_id.value_or(0),
                .Merit         = yaml::resolveEnum(source.merit),
                .AddType       = yaml::resolveFlags(source.add_type),
                .Content       = yaml::resolveEnum(source.content),
            };

            if (!source.target)
            {
                record.Target = xi::ActionTarget::Self;
            }

            if (record.Target == xi::ActionTarget::None)
            {
                throw std::runtime_error(fmt::format("{} needs a target", name));
            }

            if (source.enmity)
            {
                record.CE = source.enmity->ce;
                record.VE = source.enmity->ve;
            }

            if (source.charges)
            {
                AbilityCharges charges{ .RecastId = source.charges->recast_id.value_or(record.RecastId) };
                if (!chargedRecasts.insert(charges.RecastId).second)
                {
                    throw std::runtime_error(fmt::format("{} declares charges for recast {} a second time", name, charges.RecastId));
                }

                for (const auto& tier : source.charges->tiers)
                {
                    if (!charges.Tiers.empty() && tier.level <= charges.Tiers.back().Level)
                    {
                        throw std::runtime_error(fmt::format("{} charge tiers must go up in level", name));
                    }

                    charges.Tiers.emplace_back(AbilityChargeTier{
                        .Level      = tier.level,
                        .MaxCharges = tier.max,
                        .ChargeTime = tier.time,
                        .Merit      = yaml::resolveEnum(tier.merit),
                    });
                }

                record.Charges = std::move(charges);
            }

            records.emplace_back(std::move(record));
        }
    }

    std::ranges::sort(records, {}, &AbilityData::Id);
    return records;
}

} // namespace xi::data::datasets::abilities
