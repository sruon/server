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

#include "instance_loader.h"
#include "zone_instance.h"

#include "entities/mob_entity.h"
#include "entities/npc_entity.h"
#include "instance.h"
#include "lua/luautils.h"

#include "utils/instanceutils.h"
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

    if (!m_PInstance->Failed())
    {
        zoneutils::LoadInstanceEntities(m_PInstance, static_cast<xi::ZoneId>(effectiveZoneId));

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
