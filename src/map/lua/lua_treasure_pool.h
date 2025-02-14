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

#ifndef _LUATREASUREPOOL_H
#define _LUATREASUREPOOL_H

#include "common/cbasetypes.h"
#include "luautils.h"
#include "treasure_pool.h"

class CTreasurePool;
class CLuaTreasurePool
{
    std::shared_ptr<CTreasurePool> m_pLuaTreasurePool;

public:
    CLuaTreasurePool(const std::shared_ptr<CTreasurePool>&);

    auto GetTreasurePool() const -> std::shared_ptr<CTreasurePool>
    {
        return m_pLuaTreasurePool;
    }

    auto addMember(CLuaBaseEntity* member) -> bool;
    auto delMember(CLuaBaseEntity* member) -> bool;

    auto flush() -> bool;
    auto addTreasure(uint16 itemID, sol::object const& arg1) -> uint8;

    static void Register();
};

#endif
