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

#pragma once

#include "enums/action/proc_add_effect.h"
#include "enums/action/proc_skillchain.h"
#include "enums/msg_basic.h"
#include <variant>

using ProcType = std::variant<ActionProcAddEffect, ActionProcSkillChain>;

//==============================================================================
// Specification Structs (Used for designated initialization)
//==============================================================================

struct ProcSpec
{
    ProcType    type;
    uint8_t     info{ 0 };
    int32_t     value{ 0 };
    MSGBASIC_ID message{ MSGBASIC_NONE };
};

namespace Procs
{
    inline auto Skillchain(ActionProcSkillChain sc, const int32_t damage, const uint8_t info = 0) -> ProcSpec
    {
        return {
            .type    = sc,
            .info    = info,
            .value   = damage,
            .message = MSGBASIC_NONE,
        };
    }

    inline auto AddEffect(ActionProcAddEffect effect, const int32_t value, const MSGBASIC_ID msg = MSGBASIC_ADD_EFFECT_DAMAGE) -> ProcSpec
    {
        return {
            .type    = effect,
            .info    = 0,
            .value   = value,
            .message = msg,
        };
    }
} // namespace Procs
