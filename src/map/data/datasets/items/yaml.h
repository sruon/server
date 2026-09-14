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
#include "data/enums/auction_category.h"
#include "data/enums/damage_type.h"
#include "data/enums/element.h"
#include "data/enums/equip_script_flag.h"
#include "data/enums/equip_slot_flag.h"
#include "data/enums/furnishing_placement.h"
#include "data/enums/item_flag.h"
#include "data/enums/item_type.h"
#include "data/enums/job.h"
#include "data/enums/latent.h"
#include "data/enums/mod.h"
#include "data/enums/pet_mod_type.h"
#include "data/enums/skill_type.h"
#include "data/enums/target_type.h"
#include "data/yaml/enum_keyed_map.h"
#include "data/yaml/enum_token.h"
#include "data/yaml/schema_annotations.h"

#include <glaze/glaze.hpp>

#include <array>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace xi::data::datasets::items::wire
{

using ModValues = yaml::EnumKeyedMap<xi::Mod, int16>;

struct Usable
{
    std::optional<std::vector<yaml::EnumToken<xi::TargetType>>> targets;
    std::optional<float>                                        activation;
    std::optional<uint16>                                       animation;
    std::optional<uint32>                                       animation_time;
    std::optional<uint8>                                        max_charges;
    std::optional<uint32>                                       use_delay;
    std::optional<uint32>                                       reuse_delay;
    std::optional<bool>                                         aoe;
};

struct Latent
{
    yaml::EnumToken<xi::Latent> condition;
    uint16                      param{};
    yaml::EnumToken<xi::Mod>    mod;
    int16                       value{};
};

struct Equipment
{
    uint8                                                                           level{};
    std::optional<uint8>                                                            item_level;
    std::optional<uint8>                                                            superior_level;
    std::optional<std::variant<std::string, std::vector<yaml::EnumToken<xi::Job>>>> jobs;
    std::optional<std::vector<yaml::EnumToken<xi::EquipSlotFlag>>>                  slots;
    std::optional<std::vector<yaml::EnumToken<xi::EquipSlotFlag>>>                  remove_slots;
    std::optional<std::vector<yaml::EnumToken<xi::EquipSlotFlag>>>                  remove_slots_look;
    std::optional<uint16>                                                           model;
    std::optional<uint8>                                                            shield_size;
    std::optional<std::vector<yaml::EnumToken<xi::EquipScriptFlag>>>                script;
    std::optional<ModValues>                                                        mods;
    std::optional<yaml::EnumKeyedMap<xi::PetModType, ModValues>>                    pet_mods;
    std::optional<std::vector<Latent>>                                              latents;
};

struct Weapon
{
    std::optional<yaml::EnumToken<xi::SkillType>>  skill;
    std::optional<uint8>                           sub_skill;
    std::optional<uint16>                          item_level_skill;
    std::optional<uint16>                          item_level_parry;
    std::optional<uint16>                          item_level_magic_accuracy;
    std::optional<yaml::EnumToken<xi::DamageType>> damage_type;
    std::optional<uint8>                           hits;
    uint16                                         delay{};
    uint16                                         damage{};
    std::optional<uint16>                          unlock_points;
};

struct Furnishing
{
    uint8                                    storage{};
    uint16                                   moghancement{};
    yaml::EnumToken<xi::Element>             element;
    uint8                                    aura{};
    yaml::EnumToken<xi::FurnishingPlacement> placement;
    std::array<uint8, 2>                     size{};
    uint16                                   height{};
};

struct Puppet
{
    std::string                                           slot;
    std::optional<yaml::EnumKeyedMap<xi::Element, uint8>> elements;
};

struct Item
{
    uint16                                                    id{};
    std::optional<std::string>                                name;
    std::optional<std::string>                                sort_name;
    std::string                                               name_jp;
    yaml::EnumToken<xi::ItemType>                             type;
    std::optional<uint16>                                     sub_id;
    std::optional<uint8>                                      stack;
    std::optional<std::vector<yaml::EnumToken<xi::ItemFlag>>> flags;
    std::optional<yaml::EnumToken<xi::AuctionCategory>>       auction;
    std::optional<uint32>                                     base_sell;
    std::optional<Usable>                                     usable;
    std::optional<Equipment>                                  equipment;
    std::optional<Weapon>                                     weapon;
    std::optional<Furnishing>                                 furnishing;
    std::optional<Puppet>                                     puppet;
};

struct Document
{
    std::map<std::string, Item> items;

    using YamlRoot = yaml::DatasetRoot<&Document::items>;
};

} // namespace xi::data::datasets::items::wire
