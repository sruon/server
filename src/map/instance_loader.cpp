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

#include "instance_loader.h"
#include "zone_instance.h"

#include "data/enums/mob_mod.h"
#include "entities/mob_entity.h"
#include "entities/npc_entity.h"
#include "instance.h"
#include "lua/luautils.h"

#include "utils/instanceutils.h"
#include "utils/mobutils.h"
#include "utils/zoneutils.h"

CInstanceLoader::CInstanceLoader(uint32 instanceid, CCharEntity* PRequester)
{
    TracyZoneScoped;

    auto   instanceData = instanceutils::GetInstanceData(instanceid);
    CZone* PZone        = zoneutils::GetZone(instanceData.instance_zone);

    if (!PZone || !((PZone->GetTypeMask() & xi::ZoneType::Instanced) != xi::ZoneType::Unknown))
    {
        ShowError("Invalid zone for instanceid: %d", instanceid);
        return;
    }

    m_PRequester = PRequester;
    m_PZone      = PZone;
    m_PInstance  = static_cast<CZoneInstance*>(PZone)->CreateInstance(instanceid);
}

CInstanceLoader::~CInstanceLoader()
{
    TracyZoneScoped;
}

auto CInstanceLoader::LoadInstance() const -> CInstance*
{
    TracyZoneScoped;

    const auto realZoneId      = m_PZone->GetID();
    const auto overlayId       = m_PInstance->overlayId();
    const auto effectiveZoneId = (overlayId != 0) ? overlayId : static_cast<uint32>(realZoneId);

    auto rset = db::preparedStmt("SELECT mobname, mobid, pos_rot, pos_x, pos_y, pos_z, "
                                 "respawntime, spawntype, dropid, mob_groups.HP, mob_groups.MP, minLevel, maxLevel, "
                                 "mob_groups.poolid, allegiance "
                                 "FROM instance_entities "
                                 "INNER JOIN mob_spawn_points ON instance_entities.id = mob_spawn_points.mobid "
                                 "INNER JOIN mob_groups ON mob_groups.groupid = mob_spawn_points.groupid AND mob_groups.zoneid = ? "
                                 "WHERE instanceid = ? "
                                 "  AND ((mob_spawn_points.mobid >> 12) & 0xFFF) = ? "
                                 "  AND NOT (pos_x = 0 AND pos_y = 0 AND pos_z = 0)",
                                 realZoneId,
                                 m_PInstance->GetID(),
                                 effectiveZoneId);

    if (!m_PInstance->Failed())
    {
        FOR_DB_MULTIPLE_RESULTS(rset)
        {
            const auto* pool = mobutils::GetMobPool(rset->get<uint32>("poolid"));
            if (pool == nullptr)
            {
                continue;
            }

            auto* PMob = new CMobEntity();

            PMob->name.insert(0, rset->get<std::string>("mobname"));
            PMob->id     = rset->get<uint32>("mobid");
            PMob->targid = static_cast<uint16>(PMob->id) & 0x0FFF;

            PMob->m_SpawnPoint.rotation = rset->get<uint8>("pos_rot");
            PMob->m_SpawnPoint.x        = rset->get<float>("pos_x");
            PMob->m_SpawnPoint.y        = rset->get<float>("pos_y");
            PMob->m_SpawnPoint.z        = rset->get<float>("pos_z");
            PMob->loc.p                 = PMob->m_SpawnPoint;

            PMob->m_RespawnTime = std::chrono::seconds(rset->get<uint32>("respawntime"));
            PMob->m_SpawnType   = rset->get<xi::SpawnType>("spawntype");
            PMob->m_DropID      = rset->get<uint32>("dropid");

            PMob->HPmodifier = rset->get<uint32>("HP");
            PMob->MPmodifier = rset->get<uint32>("MP");

            PMob->m_minLevel = rset->get<uint8>("minLevel");
            PMob->m_maxLevel = rset->get<uint8>("maxLevel");

            mobutils::ApplyPool(PMob, *pool);

            PMob->allegiance  = rset->get<xi::Allegiance>("allegiance");
            PMob->m_roamFlags = pool->RoamFlags;

            // If a special instanced mob aggros, it should always aggro regardless of level.
            if ((PMob->m_Type & xi::MobType::Event) != xi::MobType::Normal)
            {
                PMob->setMobMod(xi::MobMod::AlwaysAggro, PMob->m_Aggro);
            }

            // must be here first to define mobmods
            mobutils::InitializeMob(PMob);
            PMob->PInstance = m_PInstance;

            m_PInstance->InsertMOB(PMob);
        }

        const uint32 zoneMin = (effectiveZoneId << 12) + 0x1000000;
        const uint32 zoneMax = zoneMin + 1024;

        rset = db::preparedStmt("SELECT npcid, name, pos_rot, pos_x, pos_y, pos_z, "
                                "flag, speed, speedsub, animation, animationsub, namevis, "
                                "status, entityFlags, look, name_prefix, widescan "
                                "FROM instance_entities INNER JOIN npc_list ON "
                                "(instance_entities.id = npc_list.npcid) "
                                "WHERE instanceid = ? AND npcid >= ? AND npcid < ?",
                                m_PInstance->GetID(),
                                zoneMin,
                                zoneMax);
        FOR_DB_MULTIPLE_RESULTS(rset)
        {
            CNpcEntity* PNpc = new CNpcEntity;
            PNpc->id         = rset->get<uint32>("npcid");
            PNpc->targid     = PNpc->id & 0xFFF;

            PNpc->name.insert(0, rset->get<std::string>("name"));

            PNpc->loc.p.rotation = rset->get<uint8>("pos_rot");
            PNpc->loc.p.x        = rset->get<float>("pos_x");
            PNpc->loc.p.y        = rset->get<float>("pos_y");
            PNpc->loc.p.z        = rset->get<float>("pos_z");
            const auto flag      = rset->get<uint32>("flag");
            PNpc->loc.p.moving   = static_cast<uint16>(flag & 0xFFFF);

            PNpc->m_TargID = flag >> 16; // "quite likely"

            PNpc->baseSpeed      = rset->get<uint8>("speed");
            PNpc->animationSpeed = rset->get<uint8>("speedsub");
            PNpc->UpdateSpeed();
            PNpc->animation    = rset->get<xi::Animation>("animation");
            PNpc->animationsub = rset->get<uint8>("animationsub");

            PNpc->namevis = rset->get<xi::NameVis>("namevis");
            PNpc->status  = rset->get<xi::Status>("status");
            PNpc->m_flags = rset->get<xi::EntityFlags>("entityFlags");

            uint16 sqlModelID[10];
            db::extractFromBlob(rset, "look", sqlModelID);
            PNpc->look = look_t(sqlModelID);

            PNpc->name_prefix = rset->get<uint8>("name_prefix");
            PNpc->setWidescan(rset->get<uint8>("widescan"));

            PNpc->PInstance = m_PInstance;

            m_PInstance->InsertNPC(PNpc);
        }

        // Cache every entity script before running any handler.
        // Entities may rely on one-another.
        m_PInstance->ForEachMob(
            [](CMobEntity* PMob)
            {
                luautils::OnEntityLoad(PMob);
            });

        m_PInstance->ForEachNpc(
            [](CNpcEntity* PNpc)
            {
                luautils::OnEntityLoad(PNpc);
            });

        // Finish setting up Mobs
        m_PInstance->ForEachMob(
            [&](CMobEntity* PMob)
            {
                luautils::OnMobInitialize(PMob);
                m_PInstance->FindPartyForMob(PMob);
                luautils::ApplyMixins(PMob);
                PMob->saveModifiers();
                PMob->saveMobModifiers();
            });

        // Finish setting up NPCs
        m_PInstance->ForEachNpc(
            [](CNpcEntity* PNpc)
            {
                luautils::OnNpcSpawn(PNpc);
            });

        // Cache Instance script (TODO: This will be done multiple times, don't do that)
        luautils::LoadLuaObjectFromFile(instanceutils::GetInstanceData(m_PInstance->GetID()).filename);

        // Finish setup
        luautils::OnInstanceCreatedCallback(m_PRequester, m_PInstance);
        luautils::OnInstanceCreated(m_PInstance);
    }

    return m_PInstance;
}
