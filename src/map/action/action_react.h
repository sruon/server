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

#include "common/cbasetypes.h"
#include "enums/action/react_kind.h"
#include "enums/msg_basic.h"

enum class ActionReactKind : unsigned char;
struct ReactSpec
{
    ActionReactKind type;
    uint8_t         info{ 0 };
    int16_t         value{ 0 };
    MSGBASIC_ID     message{ MSGBASIC_NONE };
};

namespace Reactions
{
    inline auto Spikes(const ActionReactKind spikeType, const int16_t damage, const MSGBASIC_ID msg = MSGBASIC_SPIKES_EFFECT_DMG) -> ReactSpec
    {
        return {
            .type    = spikeType,
            .info    = 0,
            .value   = damage,
            .message = msg,
        };
    }

    // Convenience helpers for specific spike types
    inline auto BlazeSpikes(const int16_t damage, const MSGBASIC_ID msg = MSGBASIC_SPIKES_EFFECT_DMG) -> ReactSpec
    {
        return Spikes(ActionReactKind::BlazeSpikes, damage, msg);
    }

    inline auto IceSpikes(int16_t damage, MSGBASIC_ID msg = MSGBASIC_SPIKES_EFFECT_DMG) -> ReactSpec
    {
        return Spikes(ActionReactKind::IceSpikes, damage, msg);
    }

    inline auto ShockSpikes(int16_t damage, MSGBASIC_ID msg = MSGBASIC_SPIKES_EFFECT_DMG) -> ReactSpec
    {
        return Spikes(ActionReactKind::ShockSpikes, damage, msg);
    }

    inline auto DreadSpikes(int16_t damage, MSGBASIC_ID msg = MSGBASIC_SPIKES_EFFECT_HP_DRAIN) -> ReactSpec
    {
        return Spikes(ActionReactKind::DreadSpikes, damage, msg);
    }

    inline auto Counter(int16_t damage, MSGBASIC_ID msg = MSGBASIC_ATTACK_COUNTERED_DAMAGE) -> ReactSpec
    {
        return {
            .type    = ActionReactKind::Counter,
            .info    = 0,
            .value   = damage,
            .message = msg,
        };
    }
} // namespace Reactions
