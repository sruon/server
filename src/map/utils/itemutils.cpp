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

#include <algorithm>
#include <array>
#include <cstring>
#include <map>
#include <unordered_map>

#include "common/database.h"
#include "common/logging.h"
#include "common/sjis.h"

#include "data/loader.h"
#include "entities/battleentity.h"
#include "enums/item_types.h"

#include "items/item_equipment.h"
#include "items/item_furnishing.h"
#include "items/item_general.h"
#include "items/item_linkshell.h"
#include "items/item_puppet.h"
#include "items/item_usable.h"
#include "items/item_weapon.h"
#include "lua/luautils.h"
#include "packets/c2s/0x02b_translate.h"

namespace
{
std::array<std::unique_ptr<CItem>, MAX_ITEMID> itemTemplates;
std::unique_ptr<CItemWeapon>                   unarmedItem;
std::unique_ptr<CItemWeapon>                   unarmedH2HItem;
} // namespace

std::array<DropList_t*, MAX_DROPID> g_pDropList; // global array of monster droplist items
std::array<LootList_t*, MAX_LOOTID> g_pLootList; // global array of BCNM lootlist items

// Translation lookup: language -> (name -> {item id, translated name})
std::map<GP_CLI_COMMAND_TRANSLATE_INDEX, std::unordered_map<std::string, std::pair<uint16, std::string>>> g_TranslateMap;

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

LootContainer::LootContainer(DropList_t* dropList)
: dropList(dropList)
{
}

void LootContainer::ForEachGroup(const std::function<void(const DropGroup_t&)>& func)
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

void LootContainer::ForEachItem(const std::function<void(const DropItem_t&)>& func)
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

DropList_t* GetDropList(uint16 DropID)
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

/************************************************************************
 *                                                                       *
 *  Load the items                                                       *
 *                                                                       *
 ************************************************************************/

void LoadItemList()
{
    const auto items = LoadItems();
    for (const auto& [id, data] : items)
    {
        if (id >= MAX_ITEMID)
        {
            continue;
        }

        std::unique_ptr<CItem> tplOwn;
        switch (data.Type)
        {
            case xi::ItemType::General:
                tplOwn = std::make_unique<CItemGeneral>(id);
                break;
            case xi::ItemType::Linkshell:
                tplOwn = std::make_unique<CItemLinkshell>(id);
                break;
            case xi::ItemType::Furnishing:
                tplOwn = std::make_unique<CItemFurnishing>(id);
                break;
            case xi::ItemType::Puppet:
                tplOwn = std::make_unique<CItemPuppet>(id);
                break;
            case xi::ItemType::Usable:
                tplOwn = std::make_unique<CItemUsable>(id);
                break;
            case xi::ItemType::Equipment:
                tplOwn = std::make_unique<CItemEquipment>(id);
                break;
            case xi::ItemType::Weapon:
                tplOwn = std::make_unique<CItemWeapon>(id);
                break;
            case xi::ItemType::Currency:
                tplOwn = std::make_unique<CItemCurrency>(id);
                break;
            default:
                tplOwn = std::make_unique<CItemGeneral>(id);
                break;
        }

        CItem* PItem = tplOwn.get();

        PItem->setName(data.Name.En);
        PItem->setStackSize(data.StackSize);
        PItem->setFlag(static_cast<ItemFlag>(static_cast<uint32>(data.Flags)));
        PItem->setAHCat(static_cast<uint8>(data.AhCategory));
        PItem->setBasePrice(data.BaseSell);

        if (auto const* w = std::get_if<xi::data::ItemWeaponData>(&data.Payload))
        {
            auto* PWeapon = static_cast<CItemWeapon*>(PItem);
            PWeapon->setSkillType(static_cast<uint8>(w->Skill));
            PWeapon->setSubSkillType(w->Subskill);
            PWeapon->setILvlSkill(w->IlvlSkill);
            PWeapon->setILvlParry(w->IlvlParry);
            PWeapon->setILvlMacc(w->IlvlMacc);
            PWeapon->setBaseDelay(static_cast<uint16>(w->Delay));
            PWeapon->setDelay(static_cast<uint16>(w->Delay));
            PWeapon->setDamage(static_cast<uint16>(w->Damage));
            PWeapon->setDmgType(static_cast<DAMAGE_TYPE>(w->DmgType));
            PWeapon->setMaxHit(w->Hit);
            PWeapon->setTotalUnlockPointsNeeded(w->UnlockPoints);

            int        dmg   = static_cast<int>(w->Damage);
            int        delay = static_cast<int>(w->Delay);
            const bool isH2H = PWeapon->getSkillType() == SKILL_HAND_TO_HAND;
            if ((dmg > 0 || isH2H) && delay > 0)
            {
                if (isH2H)
                {
                    delay -= 240; // base h2h delay per fist is 240 when used in DPS calculation
                    dmg += 3;     // base h2h damage
                }

                double dps = (dmg * 60.0) / delay;
                dps        = round(dps * 100) / 100;
                PWeapon->setDPS(dps);
            }
        }

        if (PItem->isType(ITEM_EQUIPMENT))
        {
            auto*       PEquipment = static_cast<CItemEquipment*>(PItem);
            const auto& e          = data.Equipment;
            uint32      jobsMask   = 0;
            for (auto j : e.Jobs)
            {
                jobsMask |= (1u << (static_cast<uint32>(j) - 1));
            }

            PEquipment->setReqLvl(e.Level);
            PEquipment->setILvl(e.Ilvl);
            PEquipment->setJobs(jobsMask);
            PEquipment->setModelId(e.ModelId);
            PEquipment->setShieldSize(static_cast<uint8>(e.ShieldSize));
            PEquipment->setEquipSlotId(static_cast<uint16>(e.Slots));
            PEquipment->setRemoveSlotId(static_cast<uint16>(e.Rslots));
            PEquipment->setRemoveSlotLookId(e.RslotLook);
            PEquipment->setSuperiorLevel(e.SuLevel);
        }

        if (PItem->isType(ITEM_USABLE) && static_cast<uint32>(data.Usable.ValidTargets) != 0)
        {
            auto* PUsable = static_cast<CItemUsable*>(PItem);
            PUsable->setValidTarget(static_cast<uint16>(data.Usable.ValidTargets));
            PUsable->setActivationTime(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<float>(data.Usable.Activation)));
            PUsable->setAnimationID(data.Usable.Animation);
            PUsable->setAnimationTime(std::chrono::seconds(data.Usable.AnimationTime));
            PUsable->setMaxCharges(data.Usable.MaxCharges);
            PUsable->setCurrentCharges(data.Usable.MaxCharges);
            PUsable->setUseDelay(std::chrono::seconds(data.Usable.UseDelay));
            PUsable->setReuseDelay(std::chrono::seconds(data.Usable.ReuseDelay));
            PUsable->setAoE(data.Usable.Aoe ? 1 : 0);
        }

        if (auto const* f = std::get_if<xi::data::ItemFurnishingData>(&data.Payload))
        {
            auto* PFurnishing = static_cast<CItemFurnishing*>(PItem);
            PFurnishing->setStorage(f->Storage);
            PFurnishing->setMoghancement(f->Moghancement);
            PFurnishing->setElement(static_cast<uint8>(f->Element));
            PFurnishing->setAura(f->Aura);
            PFurnishing->setSize(f->SizeX, f->SizeY);
            PFurnishing->setHeight(f->Height);
            PFurnishing->setPlacement(static_cast<FurnishingPlacement>(f->Placement));
        }

        if (auto const* p = std::get_if<xi::data::ItemPuppetData>(&data.Payload))
        {
            auto* PPuppet = static_cast<CItemPuppet*>(PItem);
            PPuppet->setEquipSlot(static_cast<uint32>(p->Slot));
            PPuppet->setElementSlots(p->Element);
        }

        if (PItem->isType(ITEM_EQUIPMENT))
        {
            auto* PEquipment = static_cast<CItemEquipment*>(PItem);
            for (auto const& m : data.Mods)
            {
                PEquipment->addModifier(CModifier(static_cast<Mod>(static_cast<uint16>(m.Id)), m.Value));
            }

            for (auto const& m : data.ModsPet)
            {
                // TODO(pet-type-alignment): xi::PetType and PetModType don't align; this cast is wrong.
                PEquipment->addPetModifier(CPetModifier(static_cast<Mod>(static_cast<uint16>(m.Id)), static_cast<PetModType>(m.PetType), m.Value));
            }

            for (auto const& l : data.Latents)
            {
                PEquipment->addLatent(static_cast<LATENT>(l.LatentId), l.LatentParam, static_cast<Mod>(static_cast<uint16>(l.Mod)), l.Value);
            }
        }

        // TODO(charged-items): the usable: block above skips type=equipment, so getValidTarget() is 0 and this never fires.
        if (PItem->isType(ITEM_EQUIPMENT) &&
            static_cast<CItemEquipment*>(PItem)->getValidTarget() != 0)
        {
            PItem->setSubType(ITEM_CHARGED);
        }

        itemTemplates[id] = std::move(tplOwn);

        auto sortname = data.Name.Sort.empty() ? data.Name.En : data.Name.Sort;
        std::ranges::transform(sortname, sortname.begin(), ::tolower);
        std::ranges::replace(sortname, '_', ' ');
        const auto jpNameSjis = encoding::utf8ToShiftJis(data.Name.Jp);
        if (!sortname.empty())
        {
            g_TranslateMap[GP_CLI_COMMAND_TRANSLATE_INDEX::English][sortname] = { id, jpNameSjis };
        }

        if (!jpNameSjis.empty())
        {
            g_TranslateMap[GP_CLI_COMMAND_TRANSLATE_INDEX::Japanese][jpNameSjis] = { id, sortname };
        }

        const auto scriptFile = fmt::format("./scripts/items/{}.lua", PItem->getName());
        luautils::CacheLuaObjectFromFile(scriptFile);

        if (PItem->isType(ITEM_PUPPET))
        {
            const auto attachmentFile = fmt::format("./scripts/actions/abilities/pets/attachments/{}.lua", PItem->getName());
            luautils::CacheLuaObjectFromFile(attachmentFile);
        }
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
    unarmedItem->setDmgType(DAMAGE_TYPE::NONE);
    unarmedItem->setSkillType(SKILL_NONE);
    unarmedItem->setDamage(3);

    unarmedH2HItem = std::make_unique<CItemWeapon>(0);
    unarmedH2HItem->setDmgType(DAMAGE_TYPE::HTH);
    unarmedH2HItem->setSkillType(SKILL_HAND_TO_HAND);
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
    unarmedItem.reset();
    unarmedH2HItem.reset();

    for (int32 DropID = 0; DropID < MAX_DROPID; ++DropID)
    {
        destroy(g_pDropList[DropID]);
        g_pDropList[DropID] = nullptr;
    }
}

auto TranslateItemName(GP_CLI_COMMAND_TRANSLATE_INDEX fromLang, GP_CLI_COMMAND_TRANSLATE_INDEX toLang, const std::string& name)
    -> std::optional<std::pair<uint16, std::string>>
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
