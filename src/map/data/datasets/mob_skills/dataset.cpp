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

#include "data/datasets/mob_skills/dataset.h"

#include "data/datasets/mob_skills/yaml.h"
#include "data/yaml/read.h"

#include <fmt/format.h>

#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace xi::data::datasets::mob_skills
{

namespace
{

constexpr float  kDefaultDistance{ 7.0f };
constexpr uint32 kDefaultAnimationTime{ 2000 };
constexpr uint32 kDefaultReadyTime{ 1500 };

auto toSkill(const std::string& key, const wire::MobSkill& source) -> MobSkillData
{
    MobSkillData record{
        .Id            = static_cast<xi::MobSkill>(source.id),
        .Name          = source.name.value_or(key),
        .Animation     = source.animation,
        .Aoe           = yaml::resolveEnum(source.aoe, xi::MobSkillAoe::Single),
        .Radius        = source.radius.value_or(0.0f),
        .Distance      = source.distance.value_or(kDefaultDistance),
        .AnimationTime = std::chrono::milliseconds(source.animation_time.value_or(kDefaultAnimationTime)),
        .ReadyTime     = std::chrono::milliseconds(source.ready_time.value_or(kDefaultReadyTime)),
        .Target        = xi::TargetType::Enemy,
        .Flags         = yaml::resolveFlags(source.flags),
        .Param         = source.param.value_or(0),
        .Knockback     = source.knockback.value_or(0),
    };

    if (source.target)
    {
        record.Target = yaml::resolveFlags(source.target);
        if (record.Target == xi::TargetType::None)
        {
            throw std::runtime_error(fmt::format("{} needs at least one target", key));
        }
    }

    for (const auto& skillchain : source.skillchain.value_or(std::vector<yaml::EnumToken<xi::SkillchainType>>{}))
    {
        record.Skillchain.emplace_back(yaml::resolveEnum(skillchain));
    }

    if (record.Skillchain.size() > 3)
    {
        throw std::runtime_error(fmt::format("{} has more than three skillchain properties", key));
    }

    if (record.Knockback > 7)
    {
        throw std::runtime_error(fmt::format("{} knockback {} is above 7", key, record.Knockback));
    }

    return record;
}

} // namespace

auto Dataset::decode(const std::string_view text) -> Records
{
    const auto document = yaml::read<YamlDocument>(text);

    Records                                 records;
    std::unordered_set<uint16>              skillIds;
    std::unordered_map<std::string, uint16> idByKey;

    for (const auto& [key, source] : document.mob_skills)
    {
        yaml::verifyNamedMapEntry<xi::MobSkill>(key, source.id);
        if (!skillIds.insert(source.id).second)
        {
            throw std::runtime_error(fmt::format("duplicate mob skill id {}", source.id));
        }

        idByKey.emplace(key, source.id);
        records.Skills.emplace_back(toSkill(key, source));
    }

    std::unordered_set<uint16> listIds;
    for (const auto& [key, source] : document.mob_skill_lists)
    {
        if (!listIds.insert(source.id).second)
        {
            throw std::runtime_error(fmt::format("duplicate mob skill list id {}", source.id));
        }

        if (source.skills.has_value() == source.abilities.has_value())
        {
            throw std::runtime_error(fmt::format("mob skill list {} needs exactly one of skills or abilities", key));
        }

        MobSkillListData list{ .Id = source.id, .Name = key };
        if (source.abilities)
        {
            list.Entries = *source.abilities;
        }
        else
        {
            for (const auto& skill : *source.skills)
            {
                const auto it = idByKey.find(skill);
                if (it == idByKey.end())
                {
                    throw std::runtime_error(fmt::format("mob skill list {} names unknown mob skill {}", key, skill));
                }

                list.Entries.emplace_back(it->second);
            }
        }

        records.Lists.emplace_back(std::move(list));
    }

    std::ranges::sort(records.Skills, {}, &MobSkillData::Id);
    std::ranges::sort(records.Lists, {}, &MobSkillListData::Id);
    return records;
}

} // namespace xi::data::datasets::mob_skills
