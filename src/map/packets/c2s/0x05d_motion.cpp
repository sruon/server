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

#include "0x05d_motion.h"

#include "entities/charentity.h"
#include "items.h"
#include "lua/luautils.h"
#include "packets/char_emotion.h"
#include "packets/message_basic.h"
#include "utils/jailutils.h"

namespace
{
    const auto isBell = [](const uint16 itemId)
    {
        return (itemId == DREAM_BELL || itemId == DREAM_BELL_P1 || itemId == LADY_BELL || itemId == LADY_BELL_P1);
    };
} // namespace

auto GP_CLI_COMMAND_MOTION::validate(MapSession* PSession, const CCharEntity* PChar) const -> PacketValidationResult
{
    return PacketValidator()
        .oneOf<Emote>(Number)
        .oneOf<EmoteMode>(Mode)
        .range("Param", Param, GP_CLI_COMMAND_MOTION_PARAM::Default, GP_CLI_COMMAND_MOTION_PARAM::Aim);
}

void GP_CLI_COMMAND_MOTION::process(MapSession* PSession, CCharEntity* PChar) const
{
    if (jailutils::InPrison(PChar))
    {
        PChar->pushPacket<CMessageBasicPacket>(PChar, PChar, 0, 0, MSGBASIC_CANNOT_USE_IN_AREA);
        return;
    }

    const auto  emoteId   = static_cast<Emote>(Number);
    auto const& emoteMode = static_cast<EmoteMode>(Mode);

    // Attempting to use bell emote without a bell.
    if (emoteId == Emote::BELL)
    {
        // This is the actual observed behavior. Even with a different weapon type equipped,
        // having a bell in the lockstyle is sufficient. On the other hand, if any other
        // weapon is lockstyle'd over an equipped bell, the emote will be rejected.
        // For what it's worth, geomancer bells don't count as a bell for this emote.

        // Look for a bell in the style.
        auto mainWeapon = PChar->styleItems[SLOT_MAIN];
        if (mainWeapon == 0)
        {
            // Nothing equipped in the style, look at what's actually equipped.
            mainWeapon = PChar->getEquip(SLOT_MAIN) != nullptr
                             ? PChar->getEquip(SLOT_MAIN)->getID()
                             : 0;
        }

        if (!isBell(mainWeapon))
        {
            return;
        }

        if (Param < static_cast<uint8_t>(GP_CLI_COMMAND_MOTION_PARAM::BellBegin) ||
            Param > static_cast<uint8_t>(GP_CLI_COMMAND_MOTION_PARAM::BellEnd))
        {
            // Invalid note.
            return;
        }
    }
    // Attempting to use locked job emote. Param is job id + 30.
    // TODO: Replace with JOB_GESTURE_XX KI check when #7671 is merged
    else if (emoteId == Emote::JOB && Param && !(PChar->jobs.unlocked & (1 << (Param - 0x1E))))
    {
        return;
    }

    PChar->loc.zone->PushPacket(PChar, CHAR_INRANGE_SELF, std::make_unique<CCharEmotionPacket>(PChar, UniqueNo, ActIndex, emoteId, emoteMode, Param));

    luautils::OnPlayerEmote(PChar, emoteId);
}