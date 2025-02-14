/*
===========================================================================

  Copyright (c) 2024 LandSandBoat Dev Teams

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

#include "lua_treasure_pool.h"

#include "common/logging.h"

#include "entities/charentity.h"
#include "entities/npcentity.h"
#include "lua_baseentity.h"
#include "packets/char_status.h"
#include "treasure_pool.h"
#include "utils/mobutils.h"
#include "zone_entities.h"

CLuaTreasurePool::CLuaTreasurePool(const std::shared_ptr<CTreasurePool>& PTreasurePool)
: m_pLuaTreasurePool(PTreasurePool)
{
    if (PTreasurePool == nullptr)
    {
        ShowError("CLuaTreasurePool created with nullptr instead of valid CTreasurePool*!");
    }
}

/************************************************************************
 *  Function: addMember()
 *  Purpose : Add a PC to a treasurye pool
 *  Example : pool:addMember(player)
 ************************************************************************/

auto CLuaTreasurePool::addMember(CLuaBaseEntity* member) -> bool
{
    if (!m_pLuaTreasurePool)
    {
        return false;
    }

    if (auto* PChar = dynamic_cast<CCharEntity*>(member->GetBaseEntity()))
    {
        if (PChar->PTreasurePool)
        {
            PChar->PTreasurePool->DelMember(PChar);
        }

        PChar->PTreasurePool = m_pLuaTreasurePool;
        PChar->PTreasurePool->AddMember(PChar);

        // Ensures the extended treasure pool mode is set on client, if needed
        PChar->pushPacket<CCharStatusPacket>(PChar);

        return true;
    }

    ShowWarning("Invalid entity type calling function (%s).", member->getName());
    return false;
}

/************************************************************************
 *  Function: delMember()
 *  Purpose : Remove a PC from a treasurye pool
 *  Example : pool:delMember(player)
 ************************************************************************/

auto CLuaTreasurePool::delMember(CLuaBaseEntity* member) -> bool
{
    if (!m_pLuaTreasurePool)
    {
        return false;
    }

    if (auto* PChar = dynamic_cast<CCharEntity*>(member->GetBaseEntity()))
    {
        if (PChar->PTreasurePool != m_pLuaTreasurePool)
        {
            ShowWarning("CTreasurePool::DelMember() - PChar PTreasurePool mismatched.");
            return false;
        }

        PChar->PTreasurePool->DelMember(PChar);
        PChar->PTreasurePool = nullptr;

        // Attempt to re-attach to Zone Pool, if any.
        if (auto* PZone = PChar->loc.zone; PZone && PZone->HasZonePool())
        {
            PZone->AddToZonePool(PChar);
        }
        else if (!PChar->PParty) // If not in a party, create a new solo pool
        {
            PChar->PTreasurePool = std::make_shared<CTreasurePool>(TREASUREPOOL_SOLO);
            PChar->PTreasurePool->AddMember(PChar);
        }
        else if (PChar->PParty)
        {
            // Attempt to reattach to first same-zone party pool that's not a SHARED type
            // or fallback to solo pool
            PChar->PParty->ReloadTreasurePool(PChar);
        }

        PChar->pushPacket<CCharStatusPacket>(PChar);
        return true;
    }

    ShowWarning("Invalid entity type calling function (%s).", member->getName());
    return false;
}

/************************************************************************
 *  Function: addTreasure()
 *  Purpose : Manually adds treasure to a treasure pool
 *  Example : pool:addTreasure(itemId, dropper)
 ************************************************************************/

auto CLuaTreasurePool::addTreasure(uint16 itemID, sol::object const& arg1) -> uint8
{
    if (itemID == 0)
    {
        ShowWarning("CTreasurePool::addTreasure() - Invalid itemID.");
        return 0;
    }

    if (m_pLuaTreasurePool != nullptr)
    {
        if ((arg1 != sol::lua_nil) && arg1.is<CLuaBaseEntity*>())
        {
            // The specified PEntity can be a Mob or NPC
            CLuaBaseEntity* PLuaBaseEntity = arg1.as<CLuaBaseEntity*>();
            CBaseEntity*    PEntity        = PLuaBaseEntity->GetBaseEntity();

            return m_pLuaTreasurePool->AddItem(itemID, PEntity);
        }

        return m_pLuaTreasurePool->AddItem(itemID, nullptr);
    }

    ShowWarning("CTreasurePool::addTreasure() - Pool is gone.");
    return 0;
}

auto CLuaTreasurePool::flush() -> bool
{
    if (m_pLuaTreasurePool)
    {
        auto futureTick = std::chrono::system_clock::now() + std::chrono::hours(24);
        m_pLuaTreasurePool->CheckItems(futureTick);

        return true;
    }

    return false;
}

//======================================================//

void CLuaTreasurePool::Register()
{
    SOL_USERTYPE("CTreasurePool", CLuaTreasurePool);

    SOL_REGISTER("addMember", CLuaTreasurePool::addMember);
    SOL_REGISTER("delMember", CLuaTreasurePool::delMember);

    SOL_REGISTER("addTreasure", CLuaTreasurePool::addTreasure);
    SOL_REGISTER("flush", CLuaTreasurePool::flush);
}

//======================================================//
