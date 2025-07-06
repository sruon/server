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

#include "0x032_item_trade_req.h"

#include "common/logging.h"
#include "common/timer.h"

#include "map_session.h"
#include "packets/c2s/validation.h"
#include "packets/message_basic.h"
#include "packets/message_system.h"
#include "packets/trade_action.h"
#include "packets/trade_request.h"
#include "trade_container.h"
#include "universal_container.h"
#include "utils/charutils.h"
#include "utils/jailutils.h"

auto GP_CLI_COMMAND_ITEM_TRADE_REQ::validate(MapSession* PSession, const CCharEntity* PChar) const -> PacketValidationResult
{
    return PacketValidator()
        .isNotMonstrosity(PChar);
}

void GP_CLI_COMMAND_ITEM_TRADE_REQ::process(MapSession* PSession, CCharEntity* PChar) const
{
    auto* PTarget = static_cast<CCharEntity*>(PChar->GetEntity(ActIndex, TYPE_PC));

    if (!PTarget || PTarget->id != UniqueNo)
    {
        return;
    }

    ShowDebug("%s initiated trade request with %s", PChar->getName(), PTarget->getName());

    // If the player is the same as the target, don't allow the trade
    if (PChar->id == PTarget->id)
    {
        PChar->pushPacket<CMessageBasicPacket>(PChar, PChar, 0, 0, MSGBASIC_CANNOT_ON_THAT_TARG);
        return;
    }

    if (distance(PChar->loc.p, PTarget->loc.p) > 6.0f) // Tested as around 6.0' on retail
    {
        ShowWarning("%s trade request with %s was blocked. They are too far away!", PChar->getName(), PTarget->getName());
        PChar->pushPacket<CTradeActionPacket>(PTarget, 0x07);
        return;
    }

    // You must either both be outside (your_id == their_id == 0),
    // or in the same moghouse by invite (your_id == their_id)
    if (PChar->m_moghouseID != PTarget->m_moghouseID)
    {
        ShowError("%s trade request with %s was blocked. They have mismatching moghouse IDs!", PChar->getName(), PTarget->getName());
        PChar->pushPacket<CTradeActionPacket>(PTarget, 0x07);
        return;
    }

    // If either player is in prison don't allow the trade.
    if (jailutils::InPrison(PChar) || jailutils::InPrison(PTarget))
    {
        ShowError("%s trade request with %s was blocked. They are in prison!", PChar->getName(), PTarget->getName());
        PChar->pushPacket<CTradeActionPacket>(PTarget, 0x07);
        return;
    }

    // If either player is crafting, don't allow the trade request.
    if (PChar->animation == ANIMATION_SYNTH || (PChar->CraftContainer && PChar->CraftContainer->getItemsCount() > 0) ||
        PTarget->animation == ANIMATION_SYNTH || (PTarget->CraftContainer && PTarget->CraftContainer->getItemsCount() > 0))
    {
        ShowError("%s trade request with %s was blocked. They are synthing!", PChar->getName(), PTarget->getName());
        PChar->pushPacket<CTradeActionPacket>(PTarget, 0x07);
        return;
    }

    // check /blockaid
    if (charutils::IsAidBlocked(PChar, PTarget))
    {
        ShowDebug("%s is blocking trades", PTarget->getName());
        // Target is blocking assistance
        PChar->pushPacket<CMessageSystemPacket>(0, 0, MsgStd::TargetIsCurrentlyBlocking);
        // Interaction was blocked
        PTarget->pushPacket<CMessageSystemPacket>(0, 0, MsgStd::BlockedByBlockaid);
        PChar->pushPacket<CTradeActionPacket>(PTarget, 0x07);
        return;
    }

    if (PTarget->TradePending.id == PChar->id)
    {
        ShowDebug("%s has already sent a trade request to %s", PChar->getName(), PTarget->getName());
        return;
    }

    if (!PTarget->UContainer->IsContainerEmpty())
    {
        PChar->pushPacket<CTradeActionPacket>(PTarget, 0x07);
        ShowDebug("%s's UContainer is not empty. %s cannot trade with them at this time", PTarget->getName(), PChar->getName());
        return;
    }

    const timer::time_point currentTime     = timer::now();
    const auto              lastTargetTrade = currentTime - PTarget->lastTradeInvite;
    if ((PTarget->TradePending.targid != 0 && lastTargetTrade < 60s) || PTarget->UContainer->GetType() == UCONTAINER_TRADE)
    {
        // Can't trade with someone who's already got a pending trade before timeout
        PChar->pushPacket<CTradeActionPacket>(PTarget, 0x07);
        return;
    }

    // This block usually doesn't trigger,
    // The client is generally forced to send a trade cancel packet via a cancel yes/no menu,
    // resulting in an outgoing 0x033 with 0x04 set to 0x01 for their old trade target, but sometimes the menu does not happen and a cancel is sent instead.
    if (PChar->TradePending.id != 0)
    {
        // Tell previous trader we don't want their business
        auto* POldTradeTarget = static_cast<CCharEntity*>(PChar->GetEntity(PChar->TradePending.id, TYPE_PC));
        if (POldTradeTarget && POldTradeTarget->id == PChar->TradePending.id)
        {
            POldTradeTarget->TradePending.clean();
            PChar->TradePending.clean();

            POldTradeTarget->pushPacket<CTradeActionPacket>(PChar, 0x07);
            PChar->pushPacket<CTradeActionPacket>(POldTradeTarget, 0x07);
            return;
        }
    }

    PChar->lastTradeInvite     = currentTime;
    PChar->TradePending.id     = UniqueNo;
    PChar->TradePending.targid = ActIndex;

    PTarget->lastTradeInvite     = currentTime;
    PTarget->TradePending.id     = PChar->id;
    PTarget->TradePending.targid = PChar->targid;
    PTarget->pushPacket<CTradeRequestPacket>(PChar);
}
