/*
===========================================================================

  Copyright (c) 2025 LandSandBoat Dev Teams

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

#include "lua/lua_char_party.h"

#include "common/lua.h"
#include "party/char_party.h"
#include "sol_bindings.h"

CLuaCharParty::CLuaCharParty(CCharParty* PParty)
: m_PLuaCharParty(PParty)
{
    if (PParty == nullptr)
    {
        ShowError("CLuaCharParty created with nullptr instead of valid CCharParty*!");
    }
}

//==========================================================//

void CLuaCharParty::Register()
{
    SOL_USERTYPE("CCharParty", CLuaCharParty);
};

std::ostream& operator<<(std::ostream& out, const CLuaCharParty& party)
{
    const std::string id = party.m_PLuaCharParty ? std::to_string(party.m_PLuaCharParty->getPartyId()) : "nullptr";
    return out << "CLuaCharParty(" << id << ")";
}

//==========================================================//
