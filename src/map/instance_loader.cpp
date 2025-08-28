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

#include <chrono>
#include <future>

#include "instance_loader.h"
#include "zone_instance.h"

#include "entities/mobentity.h"
#include "entities/npcentity.h"
#include "instance.h"
#include "items/item_weapon.h"
#include "lua/luautils.h"
#include "map_engine.h"
#include "mob_modifier.h"
#include "mob_spell_list.h"
#include "zone_entities.h"

#include "utils/instanceutils.h"
#include "utils/mobutils.h"
#include "utils/zoneutils.h"

CInstanceLoader::CInstanceLoader(uint32 instanceid, CCharEntity* PRequester)
{
    TracyZoneScoped;

    auto   instanceData = instanceutils::GetInstanceData(instanceid);
    CZone* PZone        = zoneutils::GetZone(instanceData.instance_zone);

    if (!PZone || !(PZone->GetTypeMask() & ZONE_TYPE::INSTANCED))
    {
        ShowError("Invalid zone for instanceid: %d", instanceid);
        return;
    }

    m_PRequester = PRequester;
    m_PZone      = PZone;
    m_PInstance  = ((CZoneInstance*)PZone)->CreateInstance(instanceid);
}

CInstanceLoader::~CInstanceLoader()
{
    TracyZoneScoped;
}

CInstance* CInstanceLoader::LoadInstance()
{
    TracyZoneScoped;

    const auto query = "SELECT mobname, mobid, pos_rot, pos_x, pos_y, pos_z, "
                       "respawntime, spawntype, dropid, mob_groups.HP, mob_groups.MP, minLevel, maxLevel, "
                       "modelid, mJob, sJob, cmbSkill, cmbDmgMult, cmbDelay, behavior, links, mobType, immunity, "
                       "ecosystemID, mobradius, speed, "
                       "STR, DEX, VIT, AGI, `INT`, MND, CHR, EVA, DEF, ATT, ACC, "
                       "slash_sdt, pierce_sdt, h2h_sdt, impact_sdt, "
                       "magical_sdt, fire_sdt, ice_sdt, wind_sdt, earth_sdt, lightning_sdt, water_sdt, light_sdt, dark_sdt, "
                       "fire_res_rank, ice_res_rank, wind_res_rank, earth_res_rank, lightning_res_rank, water_res_rank, light_res_rank, dark_res_rank, "
                       "Element, mob_pools.familyid, name_prefix, entityFlags, animationsub, "
                       "(mob_family_system.HP / 100), (mob_family_system.MP / 100), hasSpellScript, spellList, mob_groups.poolid, "
                       "allegiance, namevis, aggro, mob_pools.skill_list_id, mob_pools.true_detection, detects, "
                       "mob_family_system.charmable "
                       "FROM instance_entities INNER JOIN mob_spawn_points ON instance_entities.id = mob_spawn_points.mobid "
                       "INNER JOIN mob_groups ON mob_groups.groupid = mob_spawn_points.groupid AND mob_groups.zoneid=((mob_spawn_points.mobid>>12)&0xFFF) "
                       "INNER JOIN mob_pools ON mob_groups.poolid = mob_pools.poolid "
                       "INNER JOIN mob_resistances ON mob_resistances.resist_id = mob_pools.resist_id "
                       "INNER JOIN mob_family_system ON mob_pools.familyid = mob_family_system.familyID "
                       "WHERE instanceid = ? AND NOT (pos_x = 0 AND pos_y = 0 AND pos_z = 0)";

    const auto rset = db::preparedStmt(query, m_PInstance->GetID());

    if (!m_PInstance->Failed())
    {
        FOR_DB_MULTIPLE_RESULTS(rset)
        {
            CMobEntity* PMob = new CMobEntity();

            PMob->name.insert(0, rset->get<std::string>("mobname"));
            PMob->id     = rset->get<uint32>("mobid");
            PMob->targid = (uint16)PMob->id & 0x0FFF;

            PMob->m_SpawnPoint.rotation = rset->get<uint8>("pos_rot");
            PMob->m_SpawnPoint.x        = rset->get<float>("pos_x");
            PMob->m_SpawnPoint.y        = rset->get<float>("pos_y");
            PMob->m_SpawnPoint.z        = rset->get<float>("pos_z");
            PMob->loc.p                 = PMob->m_SpawnPoint;

            PMob->m_RespawnTime = std::chrono::seconds(rset->get<uint32>("respawntime"));
            PMob->m_SpawnType   = static_cast<SPAWNTYPE>(rset->get<int32>("spawntype"));
            PMob->m_DropID      = rset->get<uint32>("dropid");

            PMob->HPmodifier = rset->get<uint32>("HP");
            PMob->MPmodifier = rset->get<uint32>("MP");

            PMob->m_minLevel = rset->get<uint8>("minLevel");
            PMob->m_maxLevel = rset->get<uint8>("maxLevel");

            uint16 sqlModelID[10];
            db::extractFromBlob(rset, "modelid", sqlModelID);
            PMob->look = look_t(sqlModelID);

            PMob->SetMJob(rset->get<int32>("mJob"));
            PMob->SetSJob(rset->get<int32>("sJob"));

            ((CItemWeapon*)PMob->m_Weapons[SLOT_MAIN])->setMaxHit(1);
            ((CItemWeapon*)PMob->m_Weapons[SLOT_MAIN])->setSkillType(rset->get<int32>("cmbSkill"));
            PMob->m_dmgMult = rset->get<uint32>("cmbDmgMult");
            ((CItemWeapon*)PMob->m_Weapons[SLOT_MAIN])->setDelay((rset->get<int32>("cmbDelay") * 1000) / 60);
            ((CItemWeapon*)PMob->m_Weapons[SLOT_MAIN])->setBaseDelay((rset->get<int32>("cmbDelay") * 1000) / 60);

            PMob->m_Behavior    = rset->get<uint16>("behavior");
            PMob->m_Link        = rset->get<uint8>("links");
            PMob->m_Type        = rset->get<uint8>("mobType");
            PMob->m_Immunity    = static_cast<IMMUNITY>(rset->get<int32>("immunity"));
            PMob->m_EcoSystem   = static_cast<ECOSYSTEM>(rset->get<int32>("ecosystemID"));
            PMob->m_ModelRadius = rset->get<float>("mobradius");

            PMob->baseSpeed      = rset->get<uint8>("speed");
            PMob->animationSpeed = rset->get<uint8>("speed");
            PMob->UpdateSpeed();

            PMob->strRank = rset->get<uint8>("STR");
            PMob->dexRank = rset->get<uint8>("DEX");
            PMob->vitRank = rset->get<uint8>("VIT");
            PMob->agiRank = rset->get<uint8>("AGI");
            PMob->intRank = rset->get<uint8>("INT");
            PMob->mndRank = rset->get<uint8>("MND");
            PMob->chrRank = rset->get<uint8>("CHR");
            PMob->evaRank = rset->get<uint8>("EVA");
            PMob->defRank = rset->get<uint8>("DEF");
            PMob->attRank = rset->get<uint8>("ATT");
            PMob->accRank = rset->get<uint8>("ACC");

            PMob->setModifier(Mod::SLASH_SDT, rset->get<int16>("slash_sdt"));
            PMob->setModifier(Mod::PIERCE_SDT, rset->get<int16>("pierce_sdt"));
            PMob->setModifier(Mod::HTH_SDT, rset->get<int16>("h2h_sdt"));
            PMob->setModifier(Mod::IMPACT_SDT, rset->get<int16>("impact_sdt"));

            PMob->setModifier(Mod::UDMGMAGIC, rset->get<int16>("magical_sdt")); // Modifier 389, base 10000 stored as signed integer. Positives signify less damage.

            PMob->setModifier(Mod::FIRE_SDT, rset->get<int16>("fire_sdt"));    // Modifier 54, base 10000 stored as signed integer. Positives signify less damage.
            PMob->setModifier(Mod::ICE_SDT, rset->get<int16>("ice_sdt"));     // Modifier 55, base 10000 stored as signed integer. Positives signify less damage.
            PMob->setModifier(Mod::WIND_SDT, rset->get<int16>("wind_sdt"));    // Modifier 56, base 10000 stored as signed integer. Positives signify less damage.
            PMob->setModifier(Mod::EARTH_SDT, rset->get<int16>("earth_sdt"));   // Modifier 57, base 10000 stored as signed integer. Positives signify less damage.
            PMob->setModifier(Mod::THUNDER_SDT, rset->get<int16>("lightning_sdt")); // Modifier 58, base 10000 stored as signed integer. Positives signify less damage.
            PMob->setModifier(Mod::WATER_SDT, rset->get<int16>("water_sdt"));   // Modifier 59, base 10000 stored as signed integer. Positives signify less damage.
            PMob->setModifier(Mod::LIGHT_SDT, rset->get<int16>("light_sdt"));   // Modifier 60, base 10000 stored as signed integer. Positives signify less damage.
            PMob->setModifier(Mod::DARK_SDT, rset->get<int16>("dark_sdt"));    // Modifier 61, base 10000 stored as signed integer. Positives signify less damage.

            PMob->setModifier(Mod::FIRE_RES_RANK, rset->get<int8>("fire_res_rank"));
            PMob->setModifier(Mod::ICE_RES_RANK, rset->get<int8>("ice_res_rank"));
            PMob->setModifier(Mod::WIND_RES_RANK, rset->get<int8>("wind_res_rank"));
            PMob->setModifier(Mod::EARTH_RES_RANK, rset->get<int8>("earth_res_rank"));
            PMob->setModifier(Mod::THUNDER_RES_RANK, rset->get<int8>("lightning_res_rank"));
            PMob->setModifier(Mod::WATER_RES_RANK, rset->get<int8>("water_res_rank"));
            PMob->setModifier(Mod::LIGHT_RES_RANK, rset->get<int8>("light_res_rank"));
            PMob->setModifier(Mod::DARK_RES_RANK, rset->get<int8>("dark_res_rank"));

            PMob->m_Element     = rset->get<uint8>("Element");
            PMob->m_Family      = rset->get<uint16>("familyid");
            PMob->m_name_prefix = rset->get<uint8>("name_prefix");
            PMob->m_flags       = rset->get<uint32>("entityFlags");

            // Special sub animation for Mob (yovra, jailer of love, phuabo)
            // yovra 1: On top/in the sky, 2: , 3: On top/in the sky
            // phuabo 1: Underwater, 2: Out of the water, 3: Goes back underwater
            PMob->animationsub = rset->get<uint32>("animationsub");

            // Setup HP / MP Stat Percentage Boost
            PMob->HPscale = rset->get<float>("(mob_family_system.HP / 100)");
            PMob->MPscale = rset->get<float>("(mob_family_system.MP / 100)");

            // TODO: Remove me
            // Check if we should be looking up scripts for this mob
            // PMob->m_HasSpellScript = rset->get<uint8>("hasSpellScript");

            PMob->m_SpellListContainer = mobSpellList::GetMobSpellList(rset->get<int32>("spellList"));

            PMob->m_Pool = rset->get<uint32>("poolid");

            PMob->allegiance = static_cast<ALLEGIANCE_TYPE>(rset->get<int32>("allegiance"));
            PMob->namevis    = rset->get<uint32>("namevis");

            uint32 aggro  = rset->get<uint32>("aggro");
            PMob->m_Aggro = aggro;

            // If a special instanced mob aggros, it should always aggro regardless of level.
            if (PMob->m_Type & MOBTYPE_EVENT)
            {
                PMob->setMobMod(MOBMOD_ALWAYS_AGGRO, aggro);
            }

            PMob->m_MobSkillList  = rset->get<uint32>("skill_list_id");
            PMob->m_TrueDetection = rset->get<uint32>("true_detection");
            PMob->setMobMod(MOBMOD_DETECTION, rset->get<uint32>("detects"));

            PMob->setMobMod(MOBMOD_CHARMABLE, rset->get<uint32>("charmable"));

            // Overwrite base family charmables depending on mob type. Disallowed mobs which should be charmable
            // can be set in in their onInitialize
            if (PMob->m_Type & MOBTYPE_EVENT || PMob->m_Type & MOBTYPE_FISHED || PMob->m_Type & MOBTYPE_BATTLEFIELD || PMob->m_Type & MOBTYPE_NOTORIOUS)
            {
                PMob->setMobMod(MOBMOD_CHARMABLE, 0);
            }

            // must be here first to define mobmods
            mobutils::InitializeMob(PMob);
            PMob->PInstance = m_PInstance;

            m_PInstance->InsertMOB(PMob);
        }

        const auto npcQuery = "SELECT npcid, name, pos_rot, pos_x, pos_y, pos_z, "
                              "flag, speed, speedsub, animation, animationsub, namevis, "
                              "status, entityFlags, look, name_prefix, widescan "
                              "FROM instance_entities INNER JOIN npc_list ON "
                              "(instance_entities.id = npc_list.npcid) "
                              "WHERE instanceid = ? AND npcid >= ? AND npcid < ?";

        uint32 zoneMin = (m_PZone->GetID() << 12) + 0x1000000;
        uint32 zoneMax = zoneMin + 1024;

        const auto npcRset = db::preparedStmt(npcQuery, m_PInstance->GetID(), zoneMin, zoneMax);

        FOR_DB_MULTIPLE_RESULTS(npcRset)
        {
                CNpcEntity* PNpc = new CNpcEntity;
                PNpc->id         = npcRset->get<uint32>("npcid");
                PNpc->targid     = PNpc->id & 0xFFF;

                PNpc->name.insert(0, npcRset->get<std::string>("name"));

                PNpc->loc.p.rotation = npcRset->get<uint8>("pos_rot");
                PNpc->loc.p.x        = npcRset->get<float>("pos_x");
                PNpc->loc.p.y        = npcRset->get<float>("pos_y");
                PNpc->loc.p.z        = npcRset->get<float>("pos_z");
                PNpc->loc.p.moving   = npcRset->get<uint16>("flag");

                PNpc->m_TargID = npcRset->get<uint32>("flag") >> 16; // "quite likely"

                PNpc->baseSpeed      = npcRset->get<uint8>("speed");
                PNpc->animationSpeed = npcRset->get<uint8>("speedsub");
                PNpc->UpdateSpeed();
                PNpc->animation    = npcRset->get<uint8>("animation");
                PNpc->animationsub = npcRset->get<uint8>("animationsub");

                PNpc->namevis = npcRset->get<uint8>("namevis");
                PNpc->status  = static_cast<STATUS_TYPE>(npcRset->get<int32>("status"));
                PNpc->m_flags = npcRset->get<uint32>("entityFlags");

                uint16 sqlModelID[10];
                db::extractFromBlob(npcRset, "look", sqlModelID);
                PNpc->look = look_t(sqlModelID);

                PNpc->name_prefix = npcRset->get<uint8>("name_prefix");
                PNpc->widescan    = npcRset->get<uint8>("widescan");

                PNpc->PInstance = m_PInstance;

                m_PInstance->InsertNPC(PNpc);
        }

        // clang-format off
        // Finish setting up Mobs
        m_PInstance->ForEachMob([&](CMobEntity* PMob)
        {
            luautils::OnMobInitialize(PMob);
            luautils::ApplyMixins(PMob);
            ((CMobEntity*)PMob)->saveModifiers();
            ((CMobEntity*)PMob)->saveMobModifiers();

            // Add to cache
            luautils::CacheLuaObjectFromFile(
                fmt::format("./scripts/zones/{}/mobs/{}.lua",
                            PMob->loc.zone->getName(),
                            PMob->getName()));
        });
        // clang-format on

        // clang-format off
        // Finish setting up NPCs
        m_PInstance->ForEachNpc([&](CNpcEntity* PNpc)
        {
            luautils::OnNpcSpawn(PNpc);

            // Add to cache
            luautils::CacheLuaObjectFromFile(
                fmt::format("./scripts/zones/{}/npcs/{}.lua",
                            PNpc->loc.zone->getName(),
                            PNpc->getName()));
        });
        // clang-format on

        // Cache Instance script (TODO: This will be done multiple times, don't do that)
        luautils::CacheLuaObjectFromFile(instanceutils::GetInstanceData(m_PInstance->GetID()).filename);

        // Finish setup
        luautils::OnInstanceCreatedCallback(m_PRequester, m_PInstance);
        luautils::OnInstanceCreated(m_PInstance);
    }

    return m_PInstance;
}
