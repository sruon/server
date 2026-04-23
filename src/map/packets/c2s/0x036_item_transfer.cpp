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

#include "0x036_item_transfer.h"

#include "entities/charentity.h"
#include "enums/msg_std.h"
#include "items/item_store.h"
#include "items/transactions/npc_trade.h"
#include "lua/luautils.h"
#include "packets/s2c/0x053_systemmes.h"
#include "status_effect_container.h"
#include "utils/synthutils.h"

namespace
{

const auto auditTrade = [](Scheduler& scheduler, CCharEntity* PChar, CBaseEntity* PNpc, uint32_t itemId, uint32_t quantity)
{
    if (settings::get<bool>("map.AUDIT_PLAYER_TRADES"))
    {
        const auto sender       = PChar->id;
        const auto senderName   = PChar->getName();
        const auto receiver     = PNpc->id;
        const auto receiverName = PNpc->getName();

        scheduler.postToWorkerThread(
            [itemId, quantity, sender, senderName, receiver, receiverName]()
            {
                const auto tradeDate = earth_time::timestamp();
                const auto query     = "INSERT INTO audit_trade(itemid, quantity, sender, sender_name, receiver, receiver_name, date) VALUES (?, ?, ?, ?, ?, ?, ?)";
                if (!db::preparedStmt(query, itemId, quantity, sender, senderName, receiver, receiverName, tradeDate))
                {
                    ShowErrorFmt("Failed to log trade transaction (item: {}, quantity: {}, sender: {}, receiver: {}, date: {})", itemId, quantity, sender, receiver, tradeDate);
                }
            });
    }
};

} // namespace

auto GP_CLI_COMMAND_ITEM_TRANSFER::validate(MapSession* PSession, const CCharEntity* PChar) const -> PacketValidationResult
{
    return PacketValidator(PChar)
        .blockedBy({ BlockedState::InEvent, BlockedState::Monstrosity })
        .range("ItemNum", this->ItemNum, 1, 9);
}

void GP_CLI_COMMAND_ITEM_TRANSFER::process(MapSession* PSession, CCharEntity* PChar) const
{
    // If PChar is invisible don't allow the trade
    if (PChar->StatusEffectContainer->HasStatusEffectByFlag(EFFECTFLAG_INVISIBLE))
    {
        PChar->pushPacket<GP_SERV_COMMAND_SYSTEMMES>(0, 0, MsgStd::CannotWhileInvisible);
        return;
    }

    CBaseEntity* PNpc = PChar->GetEntity(this->ActIndex, TYPE_NPC | TYPE_MOB);

    // NPC must match UniqueNo and be within 6.0' of the player
    if (!PNpc ||
        PNpc->id != this->UniqueNo ||
        distance(PChar->loc.p, PNpc->loc.p) > 6.0f)
    {
        return;
    }

    // Only allow trading with mobs if it's status is an NPC
    if (PNpc->objtype == TYPE_MOB && PNpc->status != STATUS_TYPE::NORMAL)
    {
        return;
    }

    // Open the NpcTradeTransaction. takeSlot moves each offered CItem into
    // tx custody. The lifetime of the tx is owned by the script
    // dispatched from luautils::OnTrade — simple trades finalize
    // inside the call; multi-tick flows that startEvent hold the
    // tx open until the event-follow-up callbacks finish.
    auto tx = NpcTradeTransaction::start(PChar, PNpc);
    if (tx == nullptr)
    {
        ShowErrorFmt("GP_CLI_COMMAND_ITEM_TRANSFER: {} could not open trade tx with NPC {}", PChar->getName(), PNpc->getName());
        return;
    }

    // Planned audit rows — only written if every takeSlot succeeds.
    // If any slot fails we removeTransaction (rollback) and discard the plan
    // so the audit log never records a trade that was rolled back.
    struct AuditEntry
    {
        uint32_t itemId{};
        uint32_t quantity{};
    };
    std::vector<AuditEntry> pendingAudit;
    pendingAudit.reserve(this->ItemNum);

    // tx slots are dense (gil doesn't consume a tx slot), so we
    // track the next free tx slot separately from the packet index.
    uint8_t nextTxSlot = 0;

    for (int32 slotId = 0; slotId < this->ItemNum; ++slotId)
    {
        const uint8_t  invSlotId = this->PropertyItemIndexTbl[slotId];
        const uint32_t quantity  = this->ItemNumTbl[slotId];

        // Inventory slot 0 is the gil currency slot in FFXI. Route it
        // to the tx's gil handling rather than takeSlot.
        if (invSlotId == 0)
        {
            if (!tx->offerGil(quantity))
            {
                ShowErrorFmt("GP_CLI_COMMAND_ITEM_TRANSFER: {} offerGil({}) failed for NPC {}", PChar->getName(), quantity, PNpc->getName());
                PChar->removeTransaction(tx.get());
                return;
            }
            continue;
        }

        CItem* PItem = PChar->getStorage(LOC_INVENTORY)->GetItem(invSlotId);

        if (PItem == nullptr || PItem->getQuantity() < quantity)
        {
            ShowErrorFmt("GP_CLI_COMMAND_ITEM_TRANSFER: {} trying to trade NPC {} with invalid item!", PChar->getName(), PNpc->getName());
            PChar->removeTransaction(tx.get());
            return;
        }

        if (ItemStore::isBusy(PItem))
        {
            ShowErrorFmt("GP_CLI_COMMAND_ITEM_TRANSFER: {} trying to trade NPC {} with busy item!", PChar->getName(), PNpc->getName());
            PChar->removeTransaction(tx.get());
            return;
        }

        if (!tx->takeSlot(nextTxSlot, invSlotId, quantity))
        {
            ShowErrorFmt("GP_CLI_COMMAND_ITEM_TRANSFER: {} takeSlot failed for NPC {}", PChar->getName(), PNpc->getName());
            PChar->removeTransaction(tx.get());
            return;
        }
        ++nextTxSlot;

        pendingAudit.push_back({ PItem->getID(), quantity });
    }

    // All slots entered custody successfully — commit the audit rows.
    for (const auto& entry : pendingAudit)
    {
        // TODO: Don't pass around Scheduler& through PSession
        auditTrade(*PSession->scheduler, PChar, PNpc, entry.itemId, entry.quantity);
    }

    luautils::OnTrade(PChar, PNpc);

    // Re-look-up the tx via activeTransaction — the raw `tx` pointer we held
    // is unsafe across the Lua call: onSuccess / onTrade may have
    // triggered a zone change or entity teardown that cleared
    // transactions, or may have already removed the tx via commit.
    //
    // If the Lua handler started an event, keep the tx open so
    // subsequent event callbacks (onEventUpdate / onEventFinish) can
    // commit or roll back. Some trade flows emit messages, grant
    // key items, or split the commit across multiple ticks; the
    // custody window must stay open until the script releases it.
    if (auto liveTx = PChar->activeTransaction<NpcTradeTransaction>())
    {
        if (!PChar->isInEvent())
        {
            PChar->removeTransaction(liveTx.get());
        }
    }
    if (PChar->isInEvent())
    {
        // Retail accurate: If the trade started an event then any current synth is a crit fail.
        if (PChar->isCrafting())
        {
            charutils::forceSynthCritFail("GP_CLI_COMMAND_ITEM_TRANSFER", PChar);
        }
    }
}
