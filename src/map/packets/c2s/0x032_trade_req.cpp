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

#include "0x032_trade_req.h"

#include "entities/charentity.h"
#include "enums/msg_std.h"
#include "items/dbox_state.h"
#include "packets/s2c/0x021_item_trade_req.h"
#include "packets/s2c/0x022_item_trade_res.h"
#include "packets/s2c/0x053_systemmes.h"
#include "utils/charutils.h"
#include "utils/jailutils.h"

auto GP_CLI_COMMAND_TRADE_REQ::validate(MapSession* PSession, const CCharEntity* PChar) const -> PacketValidationResult
{
    return PacketValidator(PChar)
        .blockedBy({ BlockedState::InEvent, BlockedState::Monstrosity })
        .mustNotEqual(PChar->id, this->UniqueNo, "Character trading with itself");
}

void GP_CLI_COMMAND_TRADE_REQ::process(MapSession* PSession, CCharEntity* PChar) const
{
    auto* PTarget = static_cast<CCharEntity*>(PChar->GetEntity(this->ActIndex, TYPE_PC));
    if (!PTarget || PTarget->id != this->UniqueNo)
    {
        return;
    }

    ShowDebug("%s initiated trade request with %s", PChar->getName(), PTarget->getName());

    // If either player is in prison don't allow the trade.
    if (jailutils::InPrison(PChar) || jailutils::InPrison(PTarget))
    {
        ShowError("%s trade request with %s was blocked. They are in prison!", PChar->getName(), PTarget->getName());
        PChar->pushPacket<GP_SERV_COMMAND_ITEM_TRADE_RES>(PTarget, GP_ITEM_TRADE_RES_KIND::ErrYouTrade);
        return;
    }

    // If either player is crafting, don't allow the trade request.
    // TODO: Not retail accurate but leaving it here for now.
    if (PChar->isCrafting() || PTarget->isCrafting())
    {
        ShowError("%s trade request with %s was blocked. They are synthing!", PChar->getName(), PTarget->getName());
        PChar->pushPacket<GP_SERV_COMMAND_ITEM_TRADE_RES>(PTarget, GP_ITEM_TRADE_RES_KIND::ErrYouTrade);

        return;
    }

    // check /blockaid
    if (charutils::IsAidBlocked(PChar, PTarget))
    {
        ShowDebug("%s is blocking trades", PTarget->getName());
        // Target is blocking assistance
        PChar->pushPacket<GP_SERV_COMMAND_SYSTEMMES>(0, 0, MsgStd::TargetIsCurrentlyBlocking);
        // Interaction was blocked
        PTarget->pushPacket<GP_SERV_COMMAND_SYSTEMMES>(0, 0, MsgStd::BlockedByBlockaid);
        PChar->pushPacket<GP_SERV_COMMAND_ITEM_TRADE_RES>(PTarget, GP_ITEM_TRADE_RES_KIND::ErrYouTrade);
        return;
    }

    if (PTarget->tradePending.id == PChar->id)
    {
        ShowDebug("%s has already sent a trade request to %s", PChar->getName(), PTarget->getName());
        return;
    }

    // Target is already trading — reject.
    if (PTarget->activePlayerTradeTx() != nullptr)
    {
        PChar->pushPacket<GP_SERV_COMMAND_ITEM_TRADE_RES>(PTarget, GP_ITEM_TRADE_RES_KIND::ErrYouTrade);
        ShowDebug("%s is already in a trade; %s cannot trade with them at this time", PTarget->getName(), PChar->getName());
        return;
    }

    const timer::time_point currentTime     = timer::now();
    const auto              lastTargetTrade = currentTime - PTarget->lastTradeInvite;
    if (PTarget->tradePending.targid != 0 && lastTargetTrade < 60s)
    {
        // Can't trade with someone who's already got a pending trade before timeout
        PChar->pushPacket<GP_SERV_COMMAND_ITEM_TRADE_RES>(PTarget, GP_ITEM_TRADE_RES_KIND::ErrYouTrade);
        return;
    }

    // This block usually doesn't trigger,
    // The client is generally forced to send a trade cancel packet via a cancel yes/no menu,
    // resulting in an outgoing 0x033 with 0x04 set to 0x01 for their old trade target, but sometimes the menu does not happen and a cancel is sent instead.
    if (PChar->tradePending.id != 0)
    {
        // Tell previous trader we don't want their business
        auto* POldTradeTarget = static_cast<CCharEntity*>(PChar->GetEntity(PChar->tradePending.id, TYPE_PC));
        if (POldTradeTarget && POldTradeTarget->id == PChar->tradePending.id)
        {
            POldTradeTarget->tradePending.clean();
            PChar->tradePending.clean();

            POldTradeTarget->pushPacket<GP_SERV_COMMAND_ITEM_TRADE_RES>(PChar, GP_ITEM_TRADE_RES_KIND::ErrYouTrade);
            PChar->pushPacket<GP_SERV_COMMAND_ITEM_TRADE_RES>(POldTradeTarget, GP_ITEM_TRADE_RES_KIND::ErrYouTrade);
            return;
        }
    }

    PChar->lastTradeInvite     = currentTime;
    PChar->tradePending.id     = this->UniqueNo;
    PChar->tradePending.targid = this->ActIndex;

    PTarget->lastTradeInvite     = currentTime;
    PTarget->tradePending.id     = PChar->id;
    PTarget->tradePending.targid = PChar->targid;
    PTarget->pushPacket<GP_SERV_COMMAND_ITEM_TRADE_REQ>(PChar);
}
