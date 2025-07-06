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

#include "0x034_item_trade_list.h"

#include "common/async.h"
#include "entities/charentity.h"
#include "items/item_linkshell.h"
#include "packets/message_standard.h"
#include "packets/trade_item.h"
#include "packets/trade_update.h"
#include "universal_container.h"

namespace
{
    const auto auditTrade = [](CCharEntity* PChar, CCharEntity* PTarget, uint32_t itemId, uint32_t quantity)
    {
        if (settings::get<bool>("map.AUDIT_PLAYER_TRADES"))
        {
            // clang-format off
            Async::getInstance()->submit([&]()
            {
                const auto sender       = PChar->id;
                const auto senderName   = PChar->getName();
                const auto receiver     = PTarget->id;
                const auto receiverName = PTarget->getName();
                const auto tradeDate    = earth_time::timestamp();
                const auto query        = "INSERT INTO audit_trade(itemid, quantity, sender, sender_name, receiver, receiver_name, date) VALUES (?, ?, ?, ?, ?, ?, ?)";
                if (!db::preparedStmt(query, itemId, quantity, sender, senderName, receiver, receiverName, tradeDate))
                {
                    ShowErrorFmt("Failed to log trade transaction (item: {}, quantity: {}, sender: {}, receiver: {}, date: {})", itemId, quantity, sender, receiver, tradeDate);
                }
            });
            // clang-format on
        }
    };
} // namespace

auto GP_CLI_COMMAND_ITEM_TRADE_LIST::validate(MapSession* PSession, const CCharEntity* PChar) const -> PacketValidationResult
{
    return PacketValidator()
        .isNotMonstrosity(PChar)
        .range("ItemNum", ItemNum, 0, 99)
        .range("TradeIndex", TradeIndex, 0, 8);
}

void GP_CLI_COMMAND_ITEM_TRADE_LIST::process(MapSession* PSession, CCharEntity* PChar) const
{
    auto* PTarget = static_cast<CCharEntity*>(PChar->GetEntity(PChar->TradePending.targid, TYPE_PC));

    if (PTarget != nullptr && PTarget->id == PChar->TradePending.id)
    {
        if (!PChar->UContainer->IsSlotEmpty(TradeIndex))
        {
            CItem* PCurrentSlotItem = PChar->UContainer->GetItem(TradeIndex);
            if (ItemNum != 0)
            {
                ShowError("GP_CLI_COMMAND_ITEM_TRADE_LIST: Player %s trying to update trade quantity of a RESERVED item! [Item: %i | Trade Slot: %i] ",
                          PChar->getName(), PCurrentSlotItem->getID(), TradeIndex);
            }
            PCurrentSlotItem->setReserve(0);
            PChar->UContainer->ClearSlot(TradeIndex);
        }

        CItem* PItem = PChar->getStorage(LOC_INVENTORY)->GetItem(ItemIndex);
        // We used to disable Rare/Ex items being added to the container, but that is handled properly else where now
        if (PItem != nullptr && PItem->getID() == ItemNo && ItemNum + PItem->getReserve() <= PItem->getQuantity())
        {
            // whoever commented above lied about ex items
            if (PItem->getFlag() & ITEM_FLAG_EX)
            {
                return;
            }

            if (PItem->isSubType(ITEM_LOCKED))
            {
                return;
            }

            // If item count is zero remove from container
            if (ItemNum > 0)
            {
                if (PItem->isType(ITEM_LINKSHELL))
                {
                    auto* PItemLinkshell  = static_cast<CItemLinkshell*>(PItem);
                    auto* PItemLinkshell1 = reinterpret_cast<CItemLinkshell*>(PChar->getEquip(SLOT_LINK1));
                    auto* PItemLinkshell2 = reinterpret_cast<CItemLinkshell*>(PChar->getEquip(SLOT_LINK2));
                    if ((!PItemLinkshell1 && !PItemLinkshell2) || ((!PItemLinkshell1 || PItemLinkshell1->GetLSID() != PItemLinkshell->GetLSID()) &&
                                                                   (!PItemLinkshell2 || PItemLinkshell2->GetLSID() != PItemLinkshell->GetLSID())))
                    {
                        PChar->pushPacket<CMessageStandardPacket>(MsgStd::LinkshellEquipBeforeUsing);
                        PItem->setReserve(0);
                        PChar->UContainer->SetItem(TradeIndex, nullptr);
                    }
                    else
                    {
                        ShowInfo("%s->%s trade updating trade slot id %d with item %s, quantity %d", PChar->getName(), PTarget->getName(),
                                 TradeIndex, PItem->getName(), ItemNum);
                        PItem->setReserve(ItemNum + PItem->getReserve());
                        PChar->UContainer->SetItem(TradeIndex, PItem);
                    }
                }
                else
                {
                    ShowInfo("%s->%s trade updating trade slot id %d with item %s, quantity %d", PChar->getName(), PTarget->getName(),
                             TradeIndex, PItem->getName(), ItemNum);
                    PItem->setReserve(ItemNum + PItem->getReserve());
                    PChar->UContainer->SetItem(TradeIndex, PItem);
                }

                auditTrade(PChar, PTarget, PItem->getID(), ItemNum);
            }
            else
            {
                ShowInfo("%s->%s trade updating trade slot id %d with item %s, quantity 0", PChar->getName(), PTarget->getName(),
                         TradeIndex, PItem->getName());
                PItem->setReserve(0);
                PChar->UContainer->SetItem(TradeIndex, nullptr);
            }
            ShowDebug("%s->%s trade pushing packet to %s", PChar->getName(), PTarget->getName(), PChar->getName());
            PChar->pushPacket<CTradeItemPacket>(PItem, TradeIndex);
            ShowDebug("%s->%s trade pushing packet to %s", PChar->getName(), PTarget->getName(), PTarget->getName());
            PTarget->pushPacket<CTradeUpdatePacket>(PItem, TradeIndex);

            PChar->UContainer->UnLock();
            PTarget->UContainer->UnLock();
        }
    }
}
