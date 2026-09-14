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

#include "data/datasets/items/dataset.h"

#include "data/datasets/items/yaml.h"
#include "data/enums/item.h"
#include "data/yaml/read.h"

#include <fmt/format.h>

#include <algorithm>
#include <array>
#include <stdexcept>
#include <utility>
#include <variant>

namespace xi::data::datasets::items
{

namespace
{

// WAR through RUN; MON never equips gear.
constexpr uint8  kJobCount = 22;
constexpr uint32 kAllJobs  = (1U << kJobCount) - 1;

constexpr std::array<std::pair<std::string_view, uint8>, 3> kPuppetSlots{ {
    { "head", 1 },
    { "frame", 2 },
    { "attachment", 3 },
} };

auto lookup(const auto& table, const std::string_view name, const std::string_view label)
{
    const auto entry = std::ranges::find_if(table,
                                            [&](const auto& candidate)
                                            {
                                                return candidate.first == name;
                                            });
    if (entry == table.end())
    {
        throw std::runtime_error(fmt::format("unknown {} '{}'", label, name));
    }

    return entry->second;
}

template <class Enum>
auto flags(const std::optional<std::vector<yaml::EnumToken<Enum>>>& tokens) -> Enum
{
    return yaml::resolveFlags(tokens);
}

auto convertUsable(const wire::Usable& source) -> ItemUsableData
{
    return {
        .ValidTargets  = flags(source.targets),
        .Activation    = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<float>(source.activation.value_or(0.0f))),
        .Animation     = source.animation.value_or(0),
        .AnimationTime = std::chrono::seconds(source.animation_time.value_or(0)),
        .MaxCharges    = source.max_charges.value_or(0),
        .UseDelay      = std::chrono::seconds(source.use_delay.value_or(0)),
        .ReuseDelay    = std::chrono::seconds(source.reuse_delay.value_or(0)),
        .Aoe           = source.aoe.value_or(false),
    };
}

auto convertEquipment(const wire::Equipment& source) -> ItemEquipmentData
{
    uint32 jobs = 0;
    if (source.jobs)
    {
        if (const auto* shortcut = std::get_if<std::string>(&*source.jobs))
        {
            if (*shortcut != "all")
            {
                throw std::runtime_error(fmt::format("jobs must be a list or 'all', got '{}'", *shortcut));
            }

            jobs = kAllJobs;
        }
        else
        {
            for (const auto& token : std::get<std::vector<yaml::EnumToken<xi::Job>>>(*source.jobs))
            {
                const auto job = std::to_underlying(yaml::resolveEnum(token));
                if (job == 0 || job > kJobCount)
                {
                    throw std::runtime_error(fmt::format("jobs cannot list '{}'", token.Value));
                }

                jobs |= 1U << (job - 1);
            }
        }
    }

    ItemEquipmentData data{
        .Level           = source.level,
        .ItemLevel       = source.item_level.value_or(0),
        .SuperiorLevel   = source.superior_level.value_or(0),
        .Jobs            = jobs,
        .Slots           = flags(source.slots),
        .RemoveSlots     = flags(source.remove_slots),
        .RemoveSlotsLook = flags(source.remove_slots_look),
        .Model           = source.model.value_or(0),
        .ShieldSize      = source.shield_size.value_or(0),
        .Script          = flags(source.script),
        .Mods            = yaml::resolveKeys(source.mods),
    };

    for (const auto& [pet, mods] : yaml::resolveKeys(source.pet_mods))
    {
        data.PetMods.emplace(pet, yaml::resolveKeys(std::optional{ mods }));
    }

    for (const auto& latent : source.latents.value_or(std::vector<wire::Latent>{}))
    {
        data.Latents.push_back({
            .Condition = yaml::resolveEnum(latent.condition),
            .Param     = latent.param,
            .Mod       = yaml::resolveEnum(latent.mod),
            .Value     = latent.value,
        });
    }

    return data;
}

auto convertWeapon(const wire::Weapon& source) -> ItemWeaponData
{
    return {
        .Skill                  = yaml::resolveEnum(source.skill),
        .SubSkill               = source.sub_skill.value_or(0),
        .ItemLevelSkill         = source.item_level_skill.value_or(0),
        .ItemLevelParry         = source.item_level_parry.value_or(0),
        .ItemLevelMagicAccuracy = source.item_level_magic_accuracy.value_or(0),
        .DamageType             = yaml::resolveEnum(source.damage_type),
        .Hits                   = source.hits.value_or(1),
        .Delay                  = source.delay,
        .Damage                 = source.damage,
        .UnlockPoints           = source.unlock_points.value_or(0),
    };
}

auto convertFurnishing(const wire::Furnishing& source) -> ItemFurnishingData
{
    return {
        .Storage      = source.storage,
        .Moghancement = source.moghancement,
        .Element      = yaml::resolveEnum(source.element),
        .Aura         = source.aura,
        .Placement    = yaml::resolveEnum(source.placement),
        .SizeX        = source.size[0],
        .SizeY        = source.size[1],
        .Height       = source.height,
    };
}

auto convertPuppet(const wire::Puppet& source) -> ItemPuppetData
{
    uint32 elementSlots = 0;
    for (const auto& [element, power] : yaml::resolveKeys(source.elements))
    {
        const auto index = std::to_underlying(element);
        if (index == 0 || power > 0xF)
        {
            throw std::runtime_error(fmt::format("puppet element power out of range for element {}", index));
        }

        elementSlots |= static_cast<uint32>(power) << ((index - 1) * 4);
    }

    return {
        .Slot         = lookup(kPuppetSlots, source.slot, "puppet slot"),
        .ElementSlots = elementSlots,
    };
}

} // namespace

auto Dataset::decode(const std::string_view text) -> Records
{
    const auto document = yaml::read<YamlDocument>(text);
    Records    records;
    records.reserve(document.items.size());

    for (const auto& [key, source] : document.items)
    {
        try
        {
            yaml::verifyNamedMapEntry<xi::Item>(key, source.id);
            const auto name = source.name.value_or(key);

            ItemData record{
                .Id       = source.id,
                .Name     = name,
                .SortName = source.sort_name.value_or(name),
                .NameJp   = source.name_jp,
                .Type     = yaml::resolveEnum(source.type),
                .SubId    = source.sub_id.value_or(0),
                .Stack    = source.stack.value_or(1),
                .Flags    = flags(source.flags),
                .Auction  = yaml::resolveEnum(source.auction),
                .BaseSell = source.base_sell.value_or(0),
            };

            if (source.usable)
            {
                record.Usable = convertUsable(*source.usable);
            }

            if (source.equipment)
            {
                record.Equipment = convertEquipment(*source.equipment);
            }

            if (source.weapon)
            {
                record.Weapon = convertWeapon(*source.weapon);
            }

            if (source.furnishing)
            {
                record.Furnishing = convertFurnishing(*source.furnishing);
            }

            if (source.puppet)
            {
                record.Puppet = convertPuppet(*source.puppet);
            }

            if (!records.try_emplace(record.Id, std::move(record)).second)
            {
                throw std::runtime_error(fmt::format("duplicate item id {}", source.id));
            }
        }
        catch (const std::exception& error)
        {
            throw std::runtime_error(fmt::format("{}: {}", key, error.what()));
        }
    }

    return records;
}

} // namespace xi::data::datasets::items
