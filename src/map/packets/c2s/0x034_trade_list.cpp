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

#include "0x034_trade_list.h"

#include "entities/charentity.h"
#include "enums/msg_std.h"
#include "items/item_linkshell.h"
#include "items/item_store.h"
#include "items/transactions/player_trade.h"
#include "packets/s2c/0x023_item_trade_list.h"
#include "packets/s2c/0x025_item_trade_mylist.h"

namespace
{

const auto auditTrade = [](Scheduler& scheduler, CCharEntity* PChar, CCharEntity* PTarget, const CItem* PItem, uint32_t ItemNum)
{
    if (settings::get<bool>("map.AUDIT_PLAYER_TRADES"))
    {
        scheduler.postToWorkerThread(
            [itemID        = PItem->getID(),
             quantity      = ItemNum,
             sender        = PChar->id,
             sender_name   = PChar->getName(),
             receiver      = PTarget->id,
             receiver_name = PTarget->getName(),
             date          = earth_time::timestamp()]()
            {
                const auto query = "INSERT INTO audit_trade(itemid, quantity, sender, sender_name, receiver, receiver_name, date) VALUES (?, ?, ?, ?, ?, ?, ?)";
                if (!db::preparedStmt(query, itemID, quantity, sender, sender_name, receiver, receiver_name, date))
                {
                    ShowErrorFmt("Failed to log trade transaction (item: {}, quantity: {}, sender: {}, receiver: {}, date: {})", itemID, quantity, sender, receiver, date);
                }
            });
    }
};

} // namespace

auto GP_CLI_COMMAND_TRADE_LIST::validate(MapSession* PSession, const CCharEntity* PChar) const -> PacketValidationResult
{
    return PacketValidator(PChar)
        .blockedBy({ BlockedState::InEvent, BlockedState::Monstrosity })
        .mustNotEqual(PChar->tradePending.id, 0, "No trade target")
        .range("TradeIndex", this->TradeIndex, 0, 8);
}

void GP_CLI_COMMAND_TRADE_LIST::process(MapSession* PSession, CCharEntity* PChar) const
{
    auto* PTarget = static_cast<CCharEntity*>(PChar->GetEntity(PChar->tradePending.targid, TYPE_PC));

    if (!PTarget ||
        PTarget->id != PChar->tradePending.id ||
        PChar->id != PTarget->tradePending.id)
    {
        ShowWarningFmt("GP_CLI_COMMAND_TRADE_LIST: Could not find trade targets.");
        return;
    }

    // Grab the active PlayerTradeTransaction. It was opened in 0x033 when both
    // sides accepted; this packet populates offer slots.
    auto tx = PChar->activeTradeTx();
    if (tx == nullptr)
    {
        ShowWarningFmt("GP_CLI_COMMAND_TRADE_LIST: No active PlayerTradeTransaction for {}", PChar->getName());
        return;
    }

    // Any offer change invalidates both sides' prior acceptance
    // (retail: editing the offer forces both to re-click accept).
    tx->clearBothAcceptances();

    // Clearing a slot (ItemNum == 0) returns the previously-offered
    // CItem to inventory.
    if (this->ItemNum == 0)
    {
        tx->releaseSlot(PChar, this->TradeIndex);
        PChar->pushPacket<GP_SERV_COMMAND_ITEM_TRADE_MYLIST>(nullptr, this->TradeIndex, 0u);
        PTarget->pushPacket<GP_SERV_COMMAND_ITEM_TRADE_LIST>(nullptr, this->TradeIndex, 0u);
        return;
    }

    CItem* PItem = PChar->getStorage(LOC_INVENTORY)->GetItem(this->ItemIndex);

    // Validate: item exists, matches requested id, isn't Ex, quantity
    // fits the stack, and isn't held by a non-trade transaction
    // (tx-owned items from other subsystems can't be offered).
    if (!PItem ||
        PItem->getID() != this->ItemNo ||
        PItem->hasFlag(ItemFlag::Exclusive) ||
        this->ItemNum > PItem->getQuantity() ||
        ItemStore::isBusy(PItem))
    {
        ShowErrorFmt("GP_CLI_COMMAND_TRADE_LIST: {} trying to add an invalid item/quantity [Item: {} | Trade Slot: {}] ",
                     PChar->getName(),
                     this->ItemNo,
                     this->TradeIndex);
        return;
    }

    if (PItem->isType(ITEM_LINKSHELL))
    {
        auto* PItemLinkshell  = static_cast<CItemLinkshell*>(PItem);
        auto* PItemLinkshell1 = reinterpret_cast<CItemLinkshell*>(PChar->getEquip(SLOT_LINK1));
        auto* PItemLinkshell2 = reinterpret_cast<CItemLinkshell*>(PChar->getEquip(SLOT_LINK2));
        if ((!PItemLinkshell1 && !PItemLinkshell2) || ((!PItemLinkshell1 || PItemLinkshell1->GetLSID() != PItemLinkshell->GetLSID()) &&
                                                       (!PItemLinkshell2 || PItemLinkshell2->GetLSID() != PItemLinkshell->GetLSID())))
        {
            PChar->pushPacket<GP_SERV_COMMAND_MESSAGE>(MsgStd::LinkshellEquipBeforeUsing);
            tx->releaseSlot(PChar, this->TradeIndex);
            return;
        }
    }

    ShowInfo("GP_CLI_COMMAND_TRADE_LIST: %s->%s trade updating trade slot id %d with item %s, quantity %d", PChar->getName(), PTarget->getName(), this->TradeIndex, PItem->getName(), this->ItemNum);

    // Move the CItem into tx custody. takeSlot internally returns any
    // prior occupant of this tx slot first, so repeated packets with
    // different qty or item update cleanly.
    if (!tx->takeSlot(PChar, this->TradeIndex, this->ItemIndex, this->ItemNum))
    {
        ShowErrorFmt("GP_CLI_COMMAND_TRADE_LIST: takeSlot failed for {} slot {}", PChar->getName(), this->TradeIndex);
        return;
    }

    // TODO: Don't pass around Scheduler& through PSession
    auditTrade(*PSession->scheduler, PChar, PTarget, PItem, this->ItemNum);

    ShowDebug("GP_CLI_COMMAND_TRADE_LIST: %s->%s trade pushing packet to %s", PChar->getName(), PTarget->getName(), PChar->getName());
    PChar->pushPacket<GP_SERV_COMMAND_ITEM_TRADE_MYLIST>(PItem, this->TradeIndex, this->ItemNum);

    ShowDebug("GP_CLI_COMMAND_TRADE_LIST: %s->%s trade pushing packet to %s", PChar->getName(), PTarget->getName(), PTarget->getName());
    PTarget->pushPacket<GP_SERV_COMMAND_ITEM_TRADE_LIST>(PItem, this->TradeIndex, this->ItemNum);
}
