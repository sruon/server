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

#include <array>
#include <cstring>

#include "common/logging.h"

#include "entities/battleentity.h"
#include "lua/luautils.h"

std::array<CItem*, MAX_ITEMID>      g_pItemList; // global array of pointers to game items
std::array<DropList_t*, MAX_DROPID> g_pDropList; // global array of monster droplist items
std::array<LootList_t*, MAX_LOOTID> g_pLootList; // global array of BCNM lootlist items

CItemWeapon* PUnarmedItem;
CItemWeapon* PUnarmedH2HItem;

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

/************************************************************************
 *                                                                       *
 *  Actually methods of working with a global collection of items        *
 *                                                                       *
 ************************************************************************/

namespace itemutils
{
    /************************************************************************
     *                                                                       *
     *  Create an empty instance of the item by ID (private method)          *
     *                                                                       *
     ************************************************************************/

    CItem* CreateItem(uint16 ItemID)
    {
        if ((ItemID >= 512) && (ItemID <= 518))
        {
            return new CItemLinkshell(ItemID);
        }

        if ((ItemID >= 472) && (ItemID <= 3583))
        {
            return new CItemGeneral(ItemID);
        }

        if (ItemID <= 4095)
        {
            return new CItemFurnishing(ItemID);
        }

        if (ItemID <= 8191)
        {
            return new CItemUsable(ItemID);
        }

        if (ItemID <= 10239)
        {
            return new CItemPuppet(ItemID);
        }

        if (ItemID <= 16383)
        {
            return new CItemEquipment(ItemID);
        }

        if (ItemID <= 24575)
        {
            return new CItemWeapon(ItemID);
        }

        if (ItemID <= 28671)
        {
            return new CItemEquipment(ItemID);
        }

        if (ItemID <= 32767)
        {
            return new CItemGeneral(ItemID);
        }

        return nullptr;
    }

    /************************************************************************
     *                                                                       *
     *  Create a new copy of the item ID                                     *
     *                                                                       *
     ************************************************************************/

    CItem* GetItem(uint16 ItemID)
    {
        if (ItemID == 0xFFFF)
        {
            return new CItemCurrency(ItemID);
        }

        if (ItemID < MAX_ITEMID && g_pItemList[ItemID] != nullptr)
        {
            if ((ItemID >= 0x0200) && (ItemID <= 0x0206))
            {
                return new CItemLinkshell(*((CItemLinkshell*)g_pItemList[ItemID]));
            }

            if ((ItemID >= 0x01D8) && (ItemID <= 0x0DFF))
            {
                return new CItemGeneral(*((CItemGeneral*)g_pItemList[ItemID]));
            }

            if (ItemID <= 0x0FFF)
            {
                return new CItemFurnishing(*((CItemFurnishing*)g_pItemList[ItemID]));
            }

            if (ItemID <= 0x1FFF)
            {
                return new CItemUsable(*((CItemUsable*)g_pItemList[ItemID]));
            }

            if (ItemID <= 0x27FF)
            {
                return new CItemPuppet(*((CItemPuppet*)g_pItemList[ItemID]));
            }

            if (ItemID <= 0x3FFF)
            {
                return new CItemEquipment(*((CItemEquipment*)g_pItemList[ItemID]));
            }

            if (ItemID <= 0x5FFF)
            {
                return new CItemWeapon(*((CItemWeapon*)g_pItemList[ItemID]));
            }

            if (ItemID <= 0x6FFF)
            {
                return new CItemEquipment(*((CItemEquipment*)g_pItemList[ItemID]));
            }

            return new CItemGeneral(*((CItemGeneral*)g_pItemList[ItemID]));
        }

        return nullptr;
    }

    /************************************************************************
     *                                                                       *
     *  Create a copy of the item                                            *
     *                                                                       *
     ************************************************************************/

    CItem* GetItem(CItem* PItem)
    {
        if (PItem == nullptr)
        {
            ShowWarning("CItem::GetItem() - PItem is null.");
            return nullptr;
        }

        if (PItem->isType(ITEM_WEAPON))
        {
            return new CItemWeapon(*((CItemWeapon*)PItem));
        }

        if (PItem->isType(ITEM_EQUIPMENT))
        {
            return new CItemEquipment(*((CItemEquipment*)PItem));
        }

        if (PItem->isType(ITEM_USABLE))
        {
            return new CItemUsable(*((CItemUsable*)PItem));
        }

        if (PItem->isType(ITEM_LINKSHELL))
        {
            return new CItemLinkshell(*((CItemLinkshell*)PItem));
        }

        if (PItem->isType(ITEM_FURNISHING))
        {
            return new CItemFurnishing(*((CItemFurnishing*)PItem));
        }

        if (PItem->isType(ITEM_PUPPET))
        {
            return new CItemPuppet(*((CItemPuppet*)PItem));
        }

        if (PItem->isType(ITEM_GENERAL))
        {
            return new CItemGeneral(*((CItemGeneral*)PItem));
        }

        if (PItem->isType(ITEM_CURRENCY))
        {
            return new CItemCurrency(*((CItemCurrency*)PItem));
        }

        return nullptr;
    }

    /************************************************************************
     *                                                                       *
     *  Get a pointer to an item (read-only)                                 *
     *                                                                       *
     ************************************************************************/

    CItem* GetItemPointer(uint16 ItemID)
    {
        if (ItemID < MAX_ITEMID)
        {
            // False positive: this is CItem*, so it's OK
            // cppcheck-suppress CastIntegerToAddressAtReturn
            return g_pItemList[ItemID];
        }
        ShowWarning("ItemID %u too big", ItemID);
        return nullptr;
    }

    /************************************************************************
     *                                                                       *
     *  True if pointer points to a read-only g_pItemList array item         *
     *                                                                       *
     ************************************************************************/

    bool IsItemPointer(CItem* item)
    {
        return g_pItemList[item->getID()] == item;
    }

    CItemWeapon* GetUnarmedItem()
    {
        return PUnarmedItem;
    }

    CItemWeapon* GetUnarmedH2HItem()
    {
        return PUnarmedH2HItem;
    }

    /************************************************************************
     *                                                                       *
     *  Get the monsters item drop list                                      *
     *                                                                       *
     ************************************************************************/

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
        const auto query = "SELECT "
                           "b.itemId, b.name, b.stackSize, b.flags, b.aH, b.BaseSell, b.subid, "
                           "u.validTargets, u.activation, u.animation, u.animationTime, u.maxCharges, u.useDelay, u.reuseDelay, u.aoe, "
                           "a.level, a.ilevel, a.jobs, a.MId, a.shieldSize, a.scriptType, a.slot, a.rslot, a.su_level, a.rslotlook, "
                           "w.skill, w.subskill, w.ilvl_skill, w.ilvl_parry, w.ilvl_macc, w.delay, w.dmg, w.dmgType, w.hit, w.unlock_points, "
                           "f.storage, f.moghancement, f.element, f.aura, "
                           "p.slot AS puppet_slot, p.element AS puppet_element "
                           "FROM item_basic AS b "
                           "LEFT JOIN item_usable AS u USING (itemId) "
                           "LEFT JOIN item_equipment AS a USING (itemId) "
                           "LEFT JOIN item_weapon AS w USING (itemId) "
                           "LEFT JOIN item_furnishing AS f USING (itemId) "
                           "LEFT JOIN item_puppet AS p USING (itemId) "
                           "WHERE itemId < ?";

        const auto rset = db::preparedStmt(query, MAX_ITEMID);
        FOR_DB_MULTIPLE_RESULTS(rset)
        {
            CItem* PItem = CreateItem(rset->get<uint32>("itemId"));

            if (PItem != nullptr)
            {
                PItem->setName(rset->get<std::string>("name"));
                PItem->setStackSize(rset->get<uint32>("stackSize"));
                PItem->setFlag(rset->get<uint32>("flags"));
                PItem->setAHCat(rset->get<uint32>("aH"));
                PItem->setBasePrice(rset->get<uint32>("BaseSell"));
                PItem->setSubID(rset->get<uint32>("subid"));

                    if (PItem->isType(ITEM_GENERAL))
                    {
                        // TODO
                    }

                if (PItem->isType(ITEM_USABLE))
                {
                    static_cast<CItemUsable*>(PItem)->setValidTarget(rset->get<uint32>("validTargets"));
                    static_cast<CItemUsable*>(PItem)->setActivationTime(std::chrono::seconds(rset->get<uint32>("activation")));
                    static_cast<CItemUsable*>(PItem)->setAnimationID(rset->get<uint32>("animation"));
                    static_cast<CItemUsable*>(PItem)->setAnimationTime(std::chrono::seconds(rset->get<uint32>("animationTime")));
                    static_cast<CItemUsable*>(PItem)->setMaxCharges(rset->get<uint32>("maxCharges"));
                    static_cast<CItemUsable*>(PItem)->setCurrentCharges(rset->get<uint32>("maxCharges"));
                    static_cast<CItemUsable*>(PItem)->setUseDelay(std::chrono::seconds(rset->get<uint32>("useDelay")));
                    static_cast<CItemUsable*>(PItem)->setReuseDelay(std::chrono::seconds(rset->get<uint32>("reuseDelay")));
                    static_cast<CItemUsable*>(PItem)->setAoE(rset->get<uint32>("aoe"));
                }
                if (PItem->isType(ITEM_PUPPET))
                {
                    static_cast<CItemPuppet*>(PItem)->setEquipSlot(rset->get<uint32>("puppet_slot"));
                    static_cast<CItemPuppet*>(PItem)->setElementSlots(rset->get<uint32>("puppet_element"));

                        // If this is a PUP attachment, load the appropriate script as well
                        auto attachmentFile = fmt::format("./scripts/actions/abilities/pets/attachments/{}.lua", PItem->getName());
                        luautils::CacheLuaObjectFromFile(attachmentFile);
                    }

                if (PItem->isType(ITEM_EQUIPMENT))
                {
                    static_cast<CItemEquipment*>(PItem)->setReqLvl(rset->get<uint32>("level"));
                    static_cast<CItemEquipment*>(PItem)->setILvl(rset->get<uint32>("ilevel"));
                    static_cast<CItemEquipment*>(PItem)->setJobs(rset->get<uint32>("jobs"));
                    static_cast<CItemEquipment*>(PItem)->setModelId(rset->get<uint32>("MId"));
                    static_cast<CItemEquipment*>(PItem)->setShieldSize(rset->get<uint32>("shieldSize"));
                    static_cast<CItemEquipment*>(PItem)->setScriptType(rset->get<uint32>("scriptType"));
                    static_cast<CItemEquipment*>(PItem)->setEquipSlotId(rset->get<uint32>("slot"));
                    static_cast<CItemEquipment*>(PItem)->setRemoveSlotId(rset->get<uint32>("rslot"));
                    static_cast<CItemEquipment*>(PItem)->setRemoveSlotLookId(rset->get<uint32>("rslotlook"));
                    static_cast<CItemEquipment*>(PItem)->setSuperiorLevel(rset->get<uint32>("su_level"));

                        if (static_cast<CItemEquipment*>(PItem)->getValidTarget() != 0)
                        {
                            ((CItemEquipment*)PItem)->setSubType(ITEM_CHARGED);
                        }
                    }

                if (PItem->isType(ITEM_WEAPON))
                {
                    static_cast<CItemWeapon*>(PItem)->setSkillType(rset->get<uint32>("skill"));
                    static_cast<CItemWeapon*>(PItem)->setSubSkillType(rset->get<uint32>("subskill"));
                    static_cast<CItemWeapon*>(PItem)->setILvlSkill(rset->get<uint32>("ilvl_skill"));
                    static_cast<CItemWeapon*>(PItem)->setILvlParry(rset->get<uint32>("ilvl_parry"));
                    static_cast<CItemWeapon*>(PItem)->setILvlMacc(rset->get<uint32>("ilvl_macc"));
                    static_cast<CItemWeapon*>(PItem)->setBaseDelay(rset->get<uint32>("delay"));
                    static_cast<CItemWeapon*>(PItem)->setDelay((rset->get<int32>("delay") * 1000) / 60);
                    static_cast<CItemWeapon*>(PItem)->setDamage(rset->get<uint32>("dmg"));
                    static_cast<CItemWeapon*>(PItem)->setDmgType(static_cast<DAMAGE_TYPE>(rset->get<int32>("dmgType")));
                    static_cast<CItemWeapon*>(PItem)->setMaxHit(rset->get<uint32>("hit"));
                    static_cast<CItemWeapon*>(PItem)->setTotalUnlockPointsNeeded(rset->get<uint32>("unlock_points"));

                    int  dmg   = rset->get<uint32>("dmg");
                    int  delay = rset->get<int32>("delay");
                        bool isH2H = static_cast<CItemWeapon*>(PItem)->getSkillType() == SKILL_HAND_TO_HAND;

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

                            static_cast<CItemWeapon*>(PItem)->setDPS(dps);
                        }
                    }

                if (PItem->isType(ITEM_FURNISHING))
                {
                    static_cast<CItemFurnishing*>(PItem)->setStorage(rset->get<uint32>("storage"));
                    static_cast<CItemFurnishing*>(PItem)->setMoghancement(rset->get<uint32>("moghancement"));
                    static_cast<CItemFurnishing*>(PItem)->setElement(rset->get<uint32>("element"));
                    static_cast<CItemFurnishing*>(PItem)->setAura(rset->get<uint32>("aura"));
                }

                    g_pItemList[PItem->getID()] = PItem;

                    auto filename = fmt::format("./scripts/items/{}.lua", PItem->getName());
                    luautils::CacheLuaObjectFromFile(filename);
            }
        }

        const auto modsQuery = "SELECT itemId, modId, value FROM item_mods WHERE itemId IN (SELECT itemId FROM item_basic LEFT JOIN item_equipment USING (itemId))";
        const auto modsRset = db::preparedStmt(modsQuery);

        FOR_DB_MULTIPLE_RESULTS(modsRset)
        {
            uint16 ItemID = modsRset->get<uint16>("itemId");
            Mod    modID  = static_cast<Mod>(modsRset->get<int32>("modId"));
            int16  value  = modsRset->get<int16>("value");

            if ((g_pItemList[ItemID] != nullptr) && g_pItemList[ItemID]->isType(ITEM_EQUIPMENT))
            {
                static_cast<CItemEquipment*>(g_pItemList[ItemID])->addModifier(CModifier(modID, value));
            }
        }

        const auto petModsQuery = "SELECT itemId, modId, value, petType FROM item_mods_pet WHERE itemId IN (SELECT itemId FROM item_basic LEFT JOIN item_equipment USING (itemId))";
        const auto petModsRset = db::preparedStmt(petModsQuery);

        FOR_DB_MULTIPLE_RESULTS(petModsRset)
        {
            uint16     ItemID  = petModsRset->get<uint16>("itemId");
            Mod        modID   = static_cast<Mod>(petModsRset->get<int32>("modId"));
            int16      value   = petModsRset->get<int16>("value");
            PetModType petType = static_cast<PetModType>(petModsRset->get<int32>("petType"));

            if ((g_pItemList[ItemID]) && g_pItemList[ItemID]->isType(ITEM_EQUIPMENT))
            {
                static_cast<CItemEquipment*>(g_pItemList[ItemID])->addPetModifier(CPetModifier(modID, petType, value));
            }
        }

        const auto latentsQuery = "SELECT itemId, modId, value, latentId, latentParam FROM item_latents WHERE itemId IN (SELECT itemId FROM item_basic LEFT "
                                  "JOIN item_equipment USING (itemId))";
        const auto latentsRset = db::preparedStmt(latentsQuery);

        FOR_DB_MULTIPLE_RESULTS(latentsRset)
        {
            uint16 ItemID      = latentsRset->get<uint16>("itemId");
            Mod    modID       = static_cast<Mod>(latentsRset->get<int32>("modId"));
            int16  value       = latentsRset->get<int16>("value");
            LATENT latentId    = static_cast<LATENT>(latentsRset->get<int32>("latentId"));
            uint16 latentParam = latentsRset->get<uint16>("latentParam");

            if ((g_pItemList[ItemID] != nullptr) && g_pItemList[ItemID]->isType(ITEM_EQUIPMENT))
            {
                static_cast<CItemEquipment*>(g_pItemList[ItemID])->addLatent(latentId, latentParam, modID, value);
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
        const auto query = "SELECT dropId, itemId, dropType, itemRate, groupId, groupRate FROM mob_droplist WHERE dropid < ?";
        const auto rset = db::preparedStmt(query, MAX_DROPID);

        FOR_DB_MULTIPLE_RESULTS(rset)
        {
            uint16 DropID = rset->get<uint16>("dropId");

            if (g_pDropList[DropID] == nullptr)
            {
                g_pDropList[DropID] = new DropList_t;
            }

            DropList_t* dropList = g_pDropList[DropID];

            uint16 ItemID   = rset->get<uint16>("itemId");
            uint8  DropType = rset->get<uint8>("dropType");
            uint16 DropRate = rset->get<uint16>("itemRate");

            if (DropType == DROP_GROUPED)
            {
                uint8  GroupId   = rset->get<uint8>("groupId");
                uint16 GroupRate = rset->get<uint16>("groupRate");
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
     *  Handles loot from NPCs that drop things into                         *
     *  the loot pool instead of adding them directly to the inventory       *
     *                                                                       *
     ************************************************************************/

    void LoadLootList()
    {
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
        LoadLootList();

        PUnarmedItem = new CItemWeapon(0);

        PUnarmedItem->setDmgType(DAMAGE_TYPE::NONE);
        PUnarmedItem->setSkillType(SKILL_NONE);
        PUnarmedItem->setDamage(3);

        PUnarmedH2HItem = new CItemWeapon(0);

        PUnarmedH2HItem->setDmgType(DAMAGE_TYPE::HTH);
        PUnarmedH2HItem->setSkillType(SKILL_HAND_TO_HAND);
        PUnarmedH2HItem->setDamage(0);
    }

    /************************************************************************
     *                                                                       *
     *  Release the list of items                                            *
     *                                                                       *
     ************************************************************************/

    void FreeItemList()
    {
        for (int32 ItemID = 0; ItemID < MAX_ITEMID; ++ItemID)
        {
            destroy(g_pItemList[ItemID]);
            g_pItemList[ItemID] = nullptr;
        }

        for (int32 DropID = 0; DropID < MAX_DROPID; ++DropID)
        {
            destroy(g_pDropList[DropID]);
            g_pDropList[DropID] = nullptr;
        }
    }
}; // namespace itemutils
