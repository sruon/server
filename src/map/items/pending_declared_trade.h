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

#include <sol/sol.hpp>

class CBaseEntity;

// PendingDeclaredTrade — server-side record of an in-flight
// declarative trade whose commit is deferred to an event finish.
// Installed by CLuaBaseEntity::dispatchDeclaredTrades at the moment
// the event starts; consumed by luautils::OnEventFinish when the
// matching csid comes back.
//
// Scripts cannot observe or mutate this struct — it lives on
// CCharEntity as a std::unique_ptr and is only touched by the trade
// pipeline in C++.
//
// `txId` is the monotonic Transaction::id() of the tx this record was
// installed against. Event-finish compares it against the currently-
// active tx's id; mismatch means the original tx committed or was
// replaced (allocator may reuse the pointer address, so pointer-eq
// compare is unsafe). Mismatch → skip the commit path.
struct PendingDeclaredTrade
{
    uint32_t                eventId{};
    CBaseEntity*            npc{};
    uint64_t                txId{};
    sol::protected_function onFinish;
};
