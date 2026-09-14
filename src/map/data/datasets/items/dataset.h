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
#include "common/types/hash_map.h"
#include "data/enums/auction_category.h"
#include "data/enums/damage_type.h"
#include "data/enums/element.h"
#include "data/enums/equip_script_flag.h"
#include "data/enums/equip_slot_flag.h"
#include "data/enums/furnishing_placement.h"
#include "data/enums/item_flag.h"
#include "data/enums/item_type.h"
#include "data/enums/latent.h"
#include "data/enums/mod.h"
#include "data/enums/pet_mod_type.h"
#include "data/enums/skill_type.h"
#include "data/enums/target_type.h"

#include <chrono>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace xi::data
{

struct ItemUsableData
{
    xi::TargetType            ValidTargets{};
    std::chrono::milliseconds Activation{};
    uint16                    Animation{};
    std::chrono::seconds      AnimationTime{};
    uint8                     MaxCharges{};
    std::chrono::seconds      UseDelay{};
    std::chrono::seconds      ReuseDelay{};
    bool                      Aoe{};
};

struct ItemLatentData
{
    xi::Latent Condition{};
    uint16     Param{};
    xi::Mod    Mod{};
    int16      Value{};
};

struct ItemEquipmentData
{
    uint8                                            Level{};
    uint8                                            ItemLevel{};
    uint8                                            SuperiorLevel{};
    uint32                                           Jobs{};
    xi::EquipSlotFlag                                Slots{};
    xi::EquipSlotFlag                                RemoveSlots{};
    xi::EquipSlotFlag                                RemoveSlotsLook{};
    uint16                                           Model{};
    uint8                                            ShieldSize{};
    xi::EquipScriptFlag                              Script{};
    HashMap<xi::Mod, int16>                          Mods{};
    HashMap<xi::PetModType, HashMap<xi::Mod, int16>> PetMods{};
    std::vector<ItemLatentData>                      Latents{};
};

struct ItemWeaponData
{
    xi::SkillType  Skill{};
    uint8          SubSkill{};
    uint16         ItemLevelSkill{};
    uint16         ItemLevelParry{};
    uint16         ItemLevelMagicAccuracy{};
    xi::DamageType DamageType{};
    uint8          Hits{};
    uint16         Delay{};
    uint16         Damage{};
    uint16         UnlockPoints{};
};

struct ItemFurnishingData
{
    uint8                   Storage{};
    uint16                  Moghancement{};
    xi::Element             Element{};
    uint8                   Aura{};
    xi::FurnishingPlacement Placement{};
    uint8                   SizeX{};
    uint8                   SizeY{};
    uint16                  Height{};
};

struct ItemPuppetData
{
    uint8  Slot{};
    uint32 ElementSlots{};
};

struct ItemData
{
    uint16                            Id{};
    std::string                       Name{};
    std::string                       SortName{};
    std::string                       NameJp{};
    xi::ItemType                      Type{};
    uint16                            SubId{};
    uint8                             Stack{};
    xi::ItemFlag                      Flags{};
    xi::AuctionCategory               Auction{};
    uint32                            BaseSell{};
    std::optional<ItemUsableData>     Usable{};
    std::optional<ItemEquipmentData>  Equipment{};
    std::optional<ItemWeaponData>     Weapon{};
    std::optional<ItemFurnishingData> Furnishing{};
    std::optional<ItemPuppetData>     Puppet{};
};

using Items = HashMap<uint16, ItemData>;

} // namespace xi::data

namespace xi::data::datasets::items::wire
{

struct Document;

}

namespace xi::data::datasets::items
{

struct Dataset
{
    using Records      = Items;
    using YamlDocument = wire::Document;

    static constexpr std::string_view kDataPath{ "items" };
    static constexpr std::string_view kTitle{ "Items" };
    static constexpr std::string_view kDescription{ "Item templates: names, flags, and the usable, equipment, weapon, furnishing and puppet blocks." };

    static auto decode(std::string_view text) -> Records;
};

} // namespace xi::data::datasets::items
