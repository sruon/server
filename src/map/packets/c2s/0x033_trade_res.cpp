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

#include "0x033_trade_res.h"

#include "entities/charentity.h"
#include "items/transactions/player_trade.h"
#include "packets/s2c/0x022_item_trade_res.h"
#include "utils/charutils.h"

namespace
{

const auto cleanTradeTargets = [](CCharEntity* PChar, CCharEntity* PTarget)
{
    PChar->tradePending.clean();
    PTarget->tradePending.clean();
};

} // namespace

auto GP_CLI_COMMAND_TRADE_RES::validate(MapSession* PSession, const CCharEntity* PChar) const -> PacketValidationResult
{
    return PacketValidator(PChar)
        .blockedBy({ BlockedState::InEvent, BlockedState::Monstrosity })
        .oneOf<GP_CLI_COMMAND_TRADE_RES_KIND>(this->Kind)
        .mustNotEqual(PChar->tradePending.targid, 0, "No pending trade target");
}

void GP_CLI_COMMAND_TRADE_RES::process(MapSession* PSession, CCharEntity* PChar) const
{
    auto* PTarget = static_cast<CCharEntity*>(PChar->GetEntity(PChar->tradePending.targid, TYPE_PC));

    if (!PTarget ||
        PChar->tradePending.id != PTarget->id ||
        PTarget->tradePending.id != PChar->id)
    {
        ShowWarningFmt("GP_CLI_COMMAND_TRADE_RES: Could not find trade targets.");
        return;
    }

    switch (static_cast<GP_CLI_COMMAND_TRADE_RES_KIND>(this->Kind))
    {
        case GP_CLI_COMMAND_TRADE_RES_KIND::Start: // request accepted
        {
            ShowDebug("GP_CLI_COMMAND_TRADE_RES: %s accepted trade request from %s", PTarget->getName(), PChar->getName());

            // Must be within 6 yalms of each other to trade.
            if (distance(PChar->loc.p, PTarget->loc.p) > 6 || PChar->m_moghouseID != PTarget->m_moghouseID)
            {
                ShowDebug("GP_CLI_COMMAND_TRADE_RES: Too far to trade");
                cleanTradeTargets(PChar, PTarget);
                return;
            }

            // PlayerTradeTransaction::start rejects if either side already has a
            // live trade / NPC trade / synth — closes the stranded-tx
            // and cross-type-coexist classes of exploit.
            if (PlayerTradeTransaction::start(PChar, PTarget) == nullptr)
            {
                ShowDebug("GP_CLI_COMMAND_TRADE_RES: tx open refused (other tx active)");
                cleanTradeTargets(PChar, PTarget);
                return;
            }

            PChar->pushPacket<GP_SERV_COMMAND_ITEM_TRADE_RES>(PTarget, static_cast<GP_ITEM_TRADE_RES_KIND>(this->Kind));
            PTarget->pushPacket<GP_SERV_COMMAND_ITEM_TRADE_RES>(PChar, static_cast<GP_ITEM_TRADE_RES_KIND>(this->Kind));
        }
        break;
        case GP_CLI_COMMAND_TRADE_RES_KIND::Cancell: // trade cancelled
        {
            ShowDebug("GP_CLI_COMMAND_TRADE_RES: %s cancelled trade with %s", PChar->getName(), PTarget->getName());

            if (auto tx = PChar->activePlayerTradeTx())
            {
                // tx exists = both sides accepted. abort() handles
                // removeTransaction + both-sides tradePending + counterparty
                // notify.
                tx->abort(PChar);
            }
            else
            {
                // Pre-accept cancel — no tx yet, just clean the
                // pending markers and tell the counterparty.
                cleanTradeTargets(PChar, PTarget);
                PTarget->pushPacket<GP_SERV_COMMAND_ITEM_TRADE_RES>(PChar, GP_ITEM_TRADE_RES_KIND::Cancell);
            }
        }
        break;
        case GP_CLI_COMMAND_TRADE_RES_KIND::Make: // trade accepted
        {
            ShowDebug("GP_CLI_COMMAND_TRADE_RES: %s accepted trade with %s", PTarget->getName(), PChar->getName());

            PTarget->pushPacket<GP_SERV_COMMAND_ITEM_TRADE_RES>(PChar, static_cast<GP_ITEM_TRADE_RES_KIND>(Kind));

            auto tx = PChar->activePlayerTradeTx();
            if (tx == nullptr)
            {
                break;
            }
            tx->setAcceptedBy(PChar);

            if (tx->bothAccepted())
            {
                // Atomic commit: doCommit pre-validates both inventories
                // before the bidirectional swap — closes the legacy
                // "first DoTrade succeeds, second fails mid-swap" hole.
                // commitAndClose() bundles commit + bilateral notify +
                // tradePending cleanup + removeTransaction.
                if (!tx->commitAndClose())
                {
                    ShowDebug("GP_CLI_COMMAND_TRADE_RES: %s->%s trade failed (pre-validate)", PChar->getName(), PTarget->getName());
                }
            }
        }
        break;
        case GP_CLI_COMMAND_TRADE_RES_KIND::MakeCancell:
        {
            // XiPackets claim this can be sent by the client, but unknown in what conditions.
            ShowDebug("GP_CLI_COMMAND_TRADE_RES: MakeCancell received from %s", PChar->getName());
        }
        break;
    }
}
