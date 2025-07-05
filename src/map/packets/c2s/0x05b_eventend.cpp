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

#include "0x05b_eventend.h"

#include "entities/charentity.h"
#include "lua/luautils.h"

auto GP_CLI_COMMAND_EVENTEND::validate(MapSession* PSession, const CCharEntity* PChar) const -> PacketValidationResult
{
    return PacketValidator()
        .isInEvent(PChar, EventPara)
        .oneOf<GP_CLI_COMMAND_EVENTEND_MODE>(Mode);
}

void GP_CLI_COMMAND_EVENTEND::process(MapSession* PSession, CCharEntity* PChar) const
{
    auto       Result  = EndPara;
    const auto EventID = EventPara;

    if (PChar->currentEvent->option != 0)
    {
        Result = PChar->currentEvent->option;
    }

    switch (static_cast<GP_CLI_COMMAND_EVENTEND_MODE>(Mode))
    {
        case GP_CLI_COMMAND_EVENTEND_MODE::EndEvent:
        {
            luautils::OnEventFinish(PChar, EventID, Result);
            // reset if this event did not initiate another event
            if (PChar->currentEvent->eventId == EventID)
            {
                PChar->endCurrentEvent();
            }
        }
        break;
        case GP_CLI_COMMAND_EVENTEND_MODE::UpdatePendingEvent:
        {
            // If optional cutscene is started, we check to see if the selected option should lock the player
            if (Result != -1 && PChar->currentEvent->hasCutsceneOption(Result))
            {
                PChar->setLocked(true);
            }
            luautils::OnEventUpdate(PChar, EventID, Result);
        }
        break;
    }
}
