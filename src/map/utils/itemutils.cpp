/*
===========================================================================

  Copyright (c) 2010-2015 Darkstar Dev Teams

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

#include "itemutils.h"

#include "map_engine.h"

#include "common/database.h"
#include "common/logging.h"
#include "common/sjis.h"

#include <common/types/hash_map.h>

#include <algorithm>
#include <array>
#include <map>

#include "data/datasets/items/dataset.h"
#include "data/enums/item_type.h"
#include "data/loader.h"
#include "entities/battle_entity.h"
#include "items/item_flowerpot.h"
#include "items/item_furnishing.h"
#include "items/item_general.h"
#include "items/item_linkshell.h"
#include "items/item_puppet.h"
#include "lua/luautils.h"
#include "packets/c2s/0x02b_translate.h"

namespace
{

std::array<std::unique_ptr<CItem>, MAX_ITEMID> itemTemplates;
std::unique_ptr<CItemWeapon>                   unarmedItem;
std::unique_ptr<CItemWeapon>                   unarmedH2HItem;

// Lowercase name and sort name -> every item carrying it.
HashMap<std::string, std::vector<uint16>> itemIdsByName;

std::array<DropList_t*, MAX_DROPID> g_pDropList; // global array of monster droplist items

// Translation lookup: language -> (name -> {item id, translated name})
std::map<GP_CLI_COMMAND_TRANSLATE_INDEX, HashMap<std::string, std::pair<uint16, std::string>>> g_TranslateMap;

} // namespace

DropItem_t::DropItem_t(uint8 DropType, uint16 ItemID, uint16 DropRate)
: DropType(DropType)
, ItemID(ItemID)
, DropRate(DropRate)
, hasFixedRate(false)
{
}

DropItem_t::DropItem_t(uint8 DropType, uint16 ItemID, uint16 DropRate, bool hasFixedRate)
: DropType(DropType)
, ItemID(ItemID)
, DropRate(DropRate)
, hasFixedRate(hasFixedRate)
{
}

DropGroup_t::DropGroup_t(uint16 GroupRate)
: GroupRate(GroupRate)
, hasFixedRate(false)
{
}

DropGroup_t::DropGroup_t(uint16 GroupRate, bool hasFixedRate)
: GroupRate(GroupRate)
, hasFixedRate(hasFixedRate)
{
}

LootContainer::LootContainer(const DropList_t* dropList)
: dropList(dropList)
{
}

void LootContainer::ForEachGroup(FnRef<void(const DropGroup_t&)> func)
{
    for (const auto& group : dropList->Groups)
    {
        func(group);
    }

    for (const auto& group : drops.Groups)
    {
        func(group);
    }
}

void LootContainer::ForEachItem(FnRef<void(const DropItem_t&)> func)
{
    for (const auto& item : dropList->Items)
    {
        func(item);
    }

    for (const auto& item : drops.Items)
    {
        func(item);
    }
}

namespace xi::items
{

// Built on first use from the loaded templates.
auto lookupIdByName(const std::string_view name) -> Maybe<uint16>
{
    static const auto byName = []
    {
        HashMap<std::string, uint16> names;
        for (const auto& item : itemTemplates)
        {
            if (!item)
            {
                continue;
            }

            const auto [entry, inserted] = names.try_emplace(item->getName(), item->getID());
            if (!inserted)
            {
                // Some items have duplicate names (PUP attachments)
                // This gets the lowest ID until the names get properly sorted
                entry->second = std::min(entry->second, item->getID());
            }
        }
        return names;
    }();

    const auto entry = byName.find(std::string{ name });
    if (entry == byName.end())
    {
        return std::nullopt;
    }

    return entry->second;
}

auto findIdsByName(const std::string_view name) -> std::vector<uint16>
{
    std::string key{ name };
    std::ranges::transform(key, key.begin(), ::tolower);

    const auto entry = itemIdsByName.find(key);
    if (entry == itemIdsByName.end())
    {
        return {};
    }

    return entry->second;
}

auto lookup(const uint16 itemId) -> const CItem*
{
    if (itemId >= MAX_ITEMID)
    {
        ShowWarning("xi::items::lookup: itemId %u too big", itemId);
        return nullptr;
    }

    return itemTemplates[itemId].get();
}

auto clone(const CItem& source) -> std::unique_ptr<CItem>
{
    if (source.isType(ITEM_WEAPON))
    {
        return std::make_unique<CItemWeapon>(static_cast<const CItemWeapon&>(source));
    }

    if (source.isType(ITEM_EQUIPMENT))
    {
        return std::make_unique<CItemEquipment>(static_cast<const CItemEquipment&>(source));
    }

    if (source.isType(ITEM_USABLE))
    {
        return std::make_unique<CItemUsable>(static_cast<const CItemUsable&>(source));
    }

    if (source.isType(ITEM_LINKSHELL))
    {
        return std::make_unique<CItemLinkshell>(static_cast<const CItemLinkshell&>(source));
    }

    // Flowerpot check has to go before Furnishing because isType is a bitwise check and flowerpots are a child class
    if (source.isType(ITEM_FLOWERPOT))
    {
        return std::make_unique<CItemFlowerpot>(static_cast<const CItemFlowerpot&>(source));
    }

    if (source.isType(ITEM_FURNISHING))
    {
        return std::make_unique<CItemFurnishing>(static_cast<const CItemFurnishing&>(source));
    }

    if (source.isType(ITEM_PUPPET))
    {
        return std::make_unique<CItemPuppet>(static_cast<const CItemPuppet&>(source));
    }

    if (source.isType(ITEM_GENERAL))
    {
        return std::make_unique<CItemGeneral>(static_cast<const CItemGeneral&>(source));
    }

    if (source.isType(ITEM_CURRENCY))
    {
        return std::make_unique<CItemCurrency>(static_cast<const CItemCurrency&>(source));
    }

    return nullptr;
}

auto spawn(uint16 itemId) -> std::unique_ptr<CItem>
{
    if (itemId == 0xFFFF)
    {
        return std::make_unique<CItemCurrency>(itemId);
    }

    if (const CItem* tpl = lookup(itemId))
    {
        return clone(*tpl);
    }

    return nullptr;
}

auto unarmed() -> CItemWeapon*
{
    return unarmedItem.get();
}

auto unarmedH2H() -> CItemWeapon*
{
    return unarmedH2HItem.get();
}

} // namespace xi::items

namespace itemutils
{

auto GetDropList(uint16 DropID) -> DropList_t*
{
    if (DropID < MAX_DROPID)
    {
        // False positive: this is DropList_t*, so it's OK
        // cppcheck-suppress CastIntegerToAddressAtReturn
        return g_pDropList[DropID];
    }
    ShowWarning("DropID %u too big", DropID);
    return nullptr;
}

namespace
{

/************************************************************************
 *                                                                       *
 *  Load the items                                                       *
 *                                                                       *
 ************************************************************************/

void LoadItemList()
{
    auto buildFromType = [](uint16 itemId, xi::ItemType itemType) -> std::unique_ptr<CItem>
    {
        switch (itemType)
        {
            case xi::ItemType::General:
            case xi::ItemType::TrustEquipment:
                return std::make_unique<CItemGeneral>(itemId);
            case xi::ItemType::Linkshell:
                return std::make_unique<CItemLinkshell>(itemId);
            case xi::ItemType::Furnishing:
                return std::make_unique<CItemFurnishing>(itemId);
            case xi::ItemType::Puppet:
                return std::make_unique<CItemPuppet>(itemId);
            case xi::ItemType::Usable:
                return std::make_unique<CItemUsable>(itemId);
            case xi::ItemType::Equipment:
                return std::make_unique<CItemEquipment>(itemId);
            case xi::ItemType::Weapon:
                return std::make_unique<CItemWeapon>(itemId);
            case xi::ItemType::Currency:
                return std::make_unique<CItemCurrency>(itemId);
            case xi::ItemType::Flowerpot:
                return std::make_unique<CItemFlowerpot>(itemId);
            default:
                ShowErrorFmt("LoadItemList({}): Unknown item type {}", itemId, static_cast<uint8>(itemType));
                return std::make_unique<CItemGeneral>(itemId);
        }
    };

    const auto addNameIndex = [](std::string name, const uint16 itemId)
    {
        std::ranges::transform(name, name.begin(), ::tolower);
        auto& ids = itemIdsByName[name];
        if (std::ranges::find(ids, itemId) == ids.end())
        {
            ids.push_back(itemId);
        }
    };

    for (const auto& [itemId, data] : xi::data::loadDataset<xi::data::datasets::items::Dataset>())
    {
        // Gil (0xFFFF) has no template; xi::items::spawn builds it directly.
        if (itemId >= MAX_ITEMID)
        {
            continue;
        }

        auto   tplOwn = buildFromType(itemId, data.Type);
        CItem* PItem  = tplOwn.get();

        PItem->setName(data.Name);
        PItem->setStackSize(data.Stack);
        PItem->setFlag(data.Flags);
        PItem->setAHCat(std::to_underlying(data.Auction));
        PItem->setBasePrice(data.BaseSell);
        PItem->setSubID(data.SubId);

        // Weapons and equipment match ITEM_USABLE as well; only the ones that declare usable data are usable.
        if (PItem->isType(ITEM_USABLE) && data.Usable)
        {
            const auto& usable  = *data.Usable;
            auto*       PUsable = static_cast<CItemUsable*>(PItem);
            PUsable->setValidTarget(usable.ValidTargets);
            PUsable->setActivationTime(usable.Activation);
            PUsable->setAnimationID(usable.Animation);
            PUsable->setAnimationTime(usable.AnimationTime);
            PUsable->setMaxCharges(usable.MaxCharges);
            PUsable->setCurrentCharges(usable.MaxCharges);
            PUsable->setUseDelay(usable.UseDelay);
            PUsable->setReuseDelay(usable.ReuseDelay);
            PUsable->setAoE(usable.Aoe);
        }

        if (PItem->isType(ITEM_PUPPET) && data.Puppet)
        {
            static_cast<CItemPuppet*>(PItem)->setEquipSlot(data.Puppet->Slot);
            static_cast<CItemPuppet*>(PItem)->setElementSlots(data.Puppet->ElementSlots);

            // If this is a PUP attachment, load the appropriate script as well
            auto attachmentFile = fmt::format("./scripts/actions/abilities/pets/attachments/{}.lua", PItem->getName());
            luautils::LoadLuaObjectFromFile(attachmentFile);
        }

        if (PItem->isType(ITEM_EQUIPMENT) && data.Equipment)
        {
            const auto& equipment  = *data.Equipment;
            auto*       PEquipment = static_cast<CItemEquipment*>(PItem);
            PEquipment->setReqLvl(equipment.Level);
            PEquipment->setILvl(equipment.ItemLevel);
            PEquipment->setJobs(equipment.Jobs);
            PEquipment->setModelId(equipment.Model);
            PEquipment->setShieldSize(equipment.ShieldSize);
            PEquipment->setScriptType(equipment.Script);
            PEquipment->setEquipSlotId(std::to_underlying(equipment.Slots));
            PEquipment->setRemoveSlotId(std::to_underlying(equipment.RemoveSlots));
            PEquipment->setRemoveSlotLookId(std::to_underlying(equipment.RemoveSlotsLook));
            PEquipment->setSuperiorLevel(equipment.SuperiorLevel);

            for (const auto& [mod, value] : equipment.Mods)
            {
                PEquipment->addModifier(CModifier(mod, value));
            }

            for (const auto& [petType, mods] : equipment.PetMods)
            {
                for (const auto& [mod, value] : mods)
                {
                    PEquipment->addPetModifier(CPetModifier(mod, petType, value));
                }
            }

            for (const auto& latent : equipment.Latents)
            {
                PEquipment->addLatent(latent.Condition, latent.Param, latent.Mod, latent.Value);
            }

            if (PEquipment->getValidTarget() != xi::TargetType::None)
            {
                PItem->setSubType(ITEM_CHARGED);
            }
        }

        if (PItem->isType(ITEM_WEAPON) && data.Weapon)
        {
            const auto& weapon  = *data.Weapon;
            auto*       PWeapon = static_cast<CItemWeapon*>(PItem);
            PWeapon->setSkillType(weapon.Skill);
            PWeapon->setSubSkillType(weapon.SubSkill);
            PWeapon->setILvlSkill(weapon.ItemLevelSkill);
            PWeapon->setILvlParry(weapon.ItemLevelParry);
            PWeapon->setILvlMacc(weapon.ItemLevelMagicAccuracy);
            PWeapon->setBaseDelay(weapon.Delay);
            PWeapon->setDelay(weapon.Delay);
            PWeapon->setDamage(weapon.Damage);
            PWeapon->setDmgType(weapon.DamageType);
            PWeapon->setMaxHit(weapon.Hits);
            PWeapon->setTotalUnlockPointsNeeded(weapon.UnlockPoints);

            int        dmg   = weapon.Damage;
            int        delay = weapon.Delay;
            const bool isH2H = weapon.Skill == xi::SkillType::HandToHand;

            if ((dmg > 0 || isH2H) && delay > 0) // avoid division by zero for items not yet implemented. Zero dmg h2h weapons don't actually have zero dmg for the purposes of DPS.
            {
                if (isH2H)
                {
                    delay -= 240; // base h2h delay per fist is 240 when used in DPS calculation. We store Delay in the database as Weapon Delay+(240*2).
                    dmg += 3;     // add 3 base damage for DPS calculation. This base damage addition appears to come from "base" h2h damage of 3.
                                  // See Ninzas +2 in polutils/bg wiki: https://www.bg-wiki.com/ffxi/Ninzas_%2B2
                                  // The DPS field is in the DAT itself and is calculated by SE as follows:
                                  // ((104+3)*60)/(81+240) = 20
                }

                // calculate DPS
                double dps = (dmg * 60.0) / delay;

                // SE seems to round at the second decimal place, see Machine Crossbow, Falcata .DAT DPS values for rounding up and down respectively.
                // https://www.bg-wiki.com/ffxi/Falcata, https://www.bg-wiki.com/ffxi/Machine_Crossbow
                dps = round(dps * 100) / 100;

                PWeapon->setDPS(dps);
            }
        }

        if (PItem->isType(ITEM_FURNISHING) && data.Furnishing)
        {
            const auto& furnishing  = *data.Furnishing;
            auto*       PFurnishing = static_cast<CItemFurnishing*>(PItem);
            PFurnishing->setStorage(furnishing.Storage);
            PFurnishing->setMoghancement(furnishing.Moghancement);
            PFurnishing->setElement(std::to_underlying(furnishing.Element));
            PFurnishing->setAura(furnishing.Aura);
            PFurnishing->setSize(furnishing.SizeX, furnishing.SizeY);
            PFurnishing->setHeight(furnishing.Height);
            PFurnishing->setPlacement(furnishing.Placement);
        }

        itemTemplates[itemId] = std::move(tplOwn);

        addNameIndex(data.Name, itemId);
        addNameIndex(data.SortName, itemId);

        // Build translation maps. English uses sortname (the short display name) with spaces to match what the client sends.
        // Apply lowercase and replace underscores with spaces.
        auto sortname = data.SortName;
        std::ranges::transform(sortname, sortname.begin(), ::tolower);
        std::ranges::replace(sortname, '_', ' ');
        auto jpNameSjis = encoding::utf8ToShiftJis(data.NameJp); // Pre-compute the Shift-JIS equivalent
        if (!sortname.empty())
        {
            g_TranslateMap[GP_CLI_COMMAND_TRANSLATE_INDEX::English][sortname] = { itemId, jpNameSjis };
        }

        if (!jpNameSjis.empty())
        {
            g_TranslateMap[GP_CLI_COMMAND_TRANSLATE_INDEX::Japanese][jpNameSjis] = { itemId, sortname };
        }

        auto filename = fmt::format("./scripts/items/{}.lua", PItem->getName());
        luautils::LoadLuaObjectFromFile(filename);
    }
}

/************************************************************************
 *                                                                       *
 *  load lists of items monsters drop                                    *
 *                                                                       *
 ************************************************************************/

void LoadDropList()
{
    const auto rset = db::preparedStmt("SELECT dropId, itemId, dropType, itemRate, groupId, groupRate "
                                       "FROM mob_droplist WHERE dropid < ?",
                                       MAX_DROPID);
    FOR_DB_MULTIPLE_RESULTS(rset)
    {
        const auto DropID = rset->get<uint16>("dropId");

        if (g_pDropList[DropID] == nullptr)
        {
            g_pDropList[DropID] = new DropList_t;
        }

        DropList_t* dropList = g_pDropList[DropID];

        auto ItemID   = rset->get<uint16>("itemId");
        auto DropType = rset->get<uint8>("dropType");
        auto DropRate = rset->get<uint16>("itemRate");

        if (DropType == DROP_GROUPED)
        {
            const auto GroupId   = rset->get<uint8>("groupId");
            auto       GroupRate = rset->get<uint16>("groupRate");
            while (GroupId > dropList->Groups.size())
            {
                dropList->Groups.emplace_back(GroupRate);
            }
            dropList->Groups[GroupId - 1].GroupRate = GroupRate; // a bit redundant but it prevents any ordering issues.
            dropList->Groups[GroupId - 1].Items.emplace_back(DropType, ItemID, DropRate);
        }
        else
        {
            dropList->Items.emplace_back(DropType, ItemID, DropRate);
        }
    }

    // Populate 0 drop list with an empty list to support mobs that only drop loot through script logic
    g_pDropList[0] = new DropList_t;
}

} // namespace

/************************************************************************
 *                                                                       *
 *  Initialization of the  game objects                                  *
 *                                                                       *
 ************************************************************************/

void Initialize()
{
    TracyZoneScoped;

    LoadItemList();
    LoadDropList();

    unarmedItem = std::make_unique<CItemWeapon>(0);
    unarmedItem->setDmgType(xi::DamageType::None);
    unarmedItem->setSkillType(xi::SkillType::None);
    unarmedItem->setDamage(3);

    unarmedH2HItem = std::make_unique<CItemWeapon>(0);
    unarmedH2HItem->setDmgType(xi::DamageType::HandToHand);
    unarmedH2HItem->setSkillType(xi::SkillType::HandToHand);
    unarmedH2HItem->setDamage(0);

    // load magian trial data AFTER items
    auto registerTrialListeners = lua["xi"]["magian"]["registerTrialListeners"];
    if (!registerTrialListeners.valid())
    {
        ShowError("xi.magians.registerTrialListeners not valid!");
    }

    ShowInfo("do_init: loading Magian trial listeners");
    registerTrialListeners();
}

/************************************************************************
 *                                                                       *
 *  Release the list of items                                            *
 *                                                                       *
 ************************************************************************/

void FreeItemList()
{
    for (auto& tpl : itemTemplates)
    {
        tpl.reset();
    }

    itemIdsByName.clear();
    unarmedItem.reset();
    unarmedH2HItem.reset();

    for (int32 DropID = 0; DropID < MAX_DROPID; ++DropID)
    {
        destroy(g_pDropList[DropID]);
        g_pDropList[DropID] = nullptr;
    }
}

auto TranslateItemName(GP_CLI_COMMAND_TRANSLATE_INDEX fromLang, GP_CLI_COMMAND_TRANSLATE_INDEX toLang, const std::string& name)
    -> Maybe<std::pair<uint16, std::string>>
{
    std::ignore = toLang; // With only EN/JP, the "from" map already stores the other language's translation.

    // Lowercase english names to match any requested capitalization
    std::string lookupName = name;
    if (fromLang == GP_CLI_COMMAND_TRANSLATE_INDEX::English)
    {
        std::ranges::transform(lookupName, lookupName.begin(), ::tolower);
    }

    const auto& fromMap = g_TranslateMap[fromLang];
    const auto  it      = fromMap.find(lookupName);
    if (it == fromMap.end())
    {
        return std::nullopt;
    }

    return it->second;
}

}; // namespace itemutils
