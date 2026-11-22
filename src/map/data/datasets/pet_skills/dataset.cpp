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

#include "data/datasets/pet_skills/dataset.h"

#include "data/datasets/pet_skills/yaml.h"
#include "data/yaml/read.h"

#include <fmt/format.h>

#include <algorithm>
#include <ranges>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace xi::data::datasets::pet_skills
{

namespace
{

auto toArea(const wire::Area& source, const std::string_view key) -> PetSkillArea
{
    if (source.sphere.has_value() == source.cone.has_value())
    {
        throw std::runtime_error(fmt::format("{} aoe needs exactly one of sphere or cone", key));
    }

    if (source.sphere)
    {
        return AoeSphereData{ .Origin = yaml::resolveEnum(source.sphere->origin), .Radius = source.sphere->radius };
    }

    const auto origin = yaml::resolveEnum(source.cone->origin);
    if (origin != xi::ConeOrigin::Front)
    {
        throw std::runtime_error(fmt::format("{} aoe cone can only open to the front", key));
    }

    return AoeConeData{ .Origin = origin, .Distance = source.cone->distance };
}

} // namespace

auto Dataset::decode(const std::string_view text) -> Records
{
    const auto document = yaml::read<YamlDocument>(text);

    Records                    records;
    std::unordered_set<uint16> ids;

    for (const auto& skills : document.pet_skills | std::views::values)
    {
        for (const auto& [key, source] : skills)
        {
            yaml::verifyNamedMapEntry<xi::PetSkill>(key, source.id);
            if (!ids.insert(source.id).second)
            {
                throw std::runtime_error(fmt::format("duplicate pet skill id {}", source.id));
            }

            const auto mobSkill              = source.mob_skill.value_or(0);
            const auto defaultFinishCategory = [&]() -> uint8
            {
                if (mobSkill != 0)
                {
                    return 11;
                }

                return 13;
            }();

            PetSkillData record{
                .Id             = static_cast<xi::PetSkill>(source.id),
                .Name           = source.name.value_or(key),
                .MobSkill       = mobSkill,
                .Animation      = source.animation,
                .Range          = source.range.value_or(3.0f),
                .Target         = xi::TargetType::Enemy,
                .AnimationTime  = std::chrono::milliseconds(source.animation_time.value_or(2000)),
                .PrepareTime    = std::chrono::milliseconds(source.prepare_time.value_or(1000)),
                .Message        = source.message.value_or(0),
                .Flags          = yaml::resolveFlags(source.flags),
                .Param          = source.param.value_or(0),
                .FinishCategory = source.finish_category.value_or(defaultFinishCategory),
                .Knockback      = source.knockback.value_or(0),
            };

            if (source.target)
            {
                record.Target = yaml::resolveFlags(source.target);
                if (record.Target == xi::TargetType::None)
                {
                    throw std::runtime_error(fmt::format("{} needs at least one target", key));
                }
            }

            if (std::to_underlying(record.Flags) > 0xFF)
            {
                throw std::runtime_error(fmt::format("{} uses a flag pet skills cannot hold", key));
            }

            if (source.aoe)
            {
                if (source.master_range)
                {
                    throw std::runtime_error(fmt::format("{} has an aoe, so master_range does not apply", key));
                }

                record.Area = toArea(*source.aoe, key);
            }

            record.MasterRange = source.master_range.value_or(0.0f);

            for (const auto& skillchain : source.skillchain.value_or(std::vector<yaml::EnumToken<xi::SkillchainType>>{}))
            {
                record.Skillchain.emplace_back(yaml::resolveEnum(skillchain));
            }

            if (record.Skillchain.size() > 3)
            {
                throw std::runtime_error(fmt::format("{} has more than three skillchain properties", key));
            }

            records.emplace_back(std::move(record));
        }
    }

    std::ranges::sort(records, {}, &PetSkillData::Id);
    return records;
}

} // namespace xi::data::datasets::pet_skills
