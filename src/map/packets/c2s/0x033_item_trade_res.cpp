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

#include "0x033_item_trade_res.h"

#include "entities/charentity.h"
#include "packets/trade_action.h"
#include "universal_container.h"
#include "utils/charutils.h"

auto GP_CLI_COMMAND_ITEM_TRADE_RES::validate(MapSession* PSession, const CCharEntity* PChar) const -> PacketValidationResult
{
    return PacketValidator()
        .isNotMonstrosity(PChar)
        .oneOf<GP_CLI_COMMAND_ITEM_TRADE_RES_KIND>(Kind);
}

void GP_CLI_COMMAND_ITEM_TRADE_RES::process(MapSession* PSession, CCharEntity* PChar) const
{
    auto* PTarget = static_cast<CCharEntity*>(PChar->GetEntity(PChar->TradePending.targid, TYPE_PC));

    if (!PTarget || PTarget->id != PChar->TradePending.targid)
    {
        return;
    }

    switch (static_cast<GP_CLI_COMMAND_ITEM_TRADE_RES_KIND>(Kind))
    {
        case GP_CLI_COMMAND_ITEM_TRADE_RES_KIND::Start:
        {
            ShowDebug("%s accepted trade request from %s", PTarget->getName(), PChar->getName());
            if (PChar->TradePending.id == PTarget->id && PTarget->TradePending.id == PChar->id)
            {
                if (PChar->UContainer->IsContainerEmpty() && PTarget->UContainer->IsContainerEmpty())
                {
                    if (distance(PChar->loc.p, PTarget->loc.p) < 6)
                    {
                        PChar->UContainer->SetType(UCONTAINER_TRADE);
                        PChar->pushPacket<CTradeActionPacket>(PTarget, Kind);

                        PTarget->UContainer->SetType(UCONTAINER_TRADE);
                        PTarget->pushPacket<CTradeActionPacket>(PChar, Kind);
                        return;
                    }
                }
                PChar->TradePending.clean();
                PTarget->TradePending.clean();

                ShowDebug("Trade: UContainer is not empty");
            }
        }
        break;
        case GP_CLI_COMMAND_ITEM_TRADE_RES_KIND::Cancel:
        {
            ShowDebug("%s cancelled trade with %s", PTarget->getName(), PChar->getName());
            if (PChar->TradePending.id == PTarget->id && PTarget->TradePending.id == PChar->id)
            {
                if (PTarget->UContainer->GetType() == UCONTAINER_TRADE)
                {
                    PTarget->UContainer->Clean();
                }
            }
            if (PChar->UContainer->GetType() == UCONTAINER_TRADE)
            {
                PChar->UContainer->Clean();
            }

            PTarget->TradePending.clean();
            PTarget->pushPacket<CTradeActionPacket>(PChar, Kind);

            PChar->TradePending.clean();
        }
        break;
        case GP_CLI_COMMAND_ITEM_TRADE_RES_KIND::Make:
        {
            ShowDebug("%s accepted trade with %s", PTarget->getName(), PChar->getName());
            if (PChar->TradePending.id == PTarget->id && PTarget->TradePending.id == PChar->id)
            {
                PChar->UContainer->SetLock();
                PTarget->pushPacket<CTradeActionPacket>(PChar, Kind);

                if (PTarget->UContainer->IsLocked())
                {
                    if (charutils::CanTrade(PChar, PTarget) && charutils::CanTrade(PTarget, PChar))
                    {
                        charutils::DoTrade(PChar, PTarget);
                        PTarget->pushPacket<CTradeActionPacket>(PTarget, 9);

                        charutils::DoTrade(PTarget, PChar);
                        PChar->pushPacket<CTradeActionPacket>(PChar, 9);
                    }
                    else
                    {
                        // Failed to trade
                        // Either players containers are full or illegal item trade attempted
                        ShowDebug("%s->%s trade failed (full inventory or illegal items)", PChar->getName(), PTarget->getName());
                        PChar->pushPacket<CTradeActionPacket>(PTarget, 1);
                        PTarget->pushPacket<CTradeActionPacket>(PChar, 1);
                    }
                    PChar->TradePending.clean();
                    PChar->UContainer->Clean();

                    PTarget->TradePending.clean();
                    PTarget->UContainer->Clean();
                }
            }
        }
        break;
        case GP_CLI_COMMAND_ITEM_TRADE_RES_KIND::MakeCancel:
        {
            // TODO: Figure out
        }
        break;
    }
}
