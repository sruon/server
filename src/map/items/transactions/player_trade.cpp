/*
===========================================================================

  Copyright (c) 2026 LandSandBoat Dev Teams

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

#include "player_trade.h"

#include "common/mmo.h"

#include "items/item.h"
#include "items/item_store.h"

#include "entities/charentity.h"
#include "item_container.h"
#include "packets/s2c/0x022_item_trade_res.h"
#include "utils/charutils.h"
#include "utils/itemutils.h"

#include <vector>

PlayerTradeTransaction::PlayerTradeTransaction(xi::Badge<PlayerTradeTransaction>, CCharEntity* initiator, CCharEntity* partner)
: Transaction()
, initiator_(initiator)
, partner_(partner)
{
}

auto PlayerTradeTransaction::start(CCharEntity* initiator, CCharEntity* partner) -> TransactionHandle<PlayerTradeTransaction>
{
    if (initiator == nullptr || partner == nullptr || initiator == partner)
    {
        return {};
    }
    // One player-to-player trade per participant. Cross-type
    // coexistence with SynthTransaction / NpcTradeTransaction is fine — moveToTransaction's
    // prior-state check is the real dup guard.
    if (initiator->activeTransaction<PlayerTradeTransaction>() != nullptr ||
        partner->activeTransaction<PlayerTradeTransaction>() != nullptr)
    {
        return {};
    }

    auto tx = std::make_unique<PlayerTradeTransaction>(xi::Badge<PlayerTradeTransaction>{}, initiator, partner);
    return initiator->addTransaction(std::move(tx));
}

auto PlayerTradeTransaction::initiator() const -> CCharEntity*
{
    return initiator_;
}

auto PlayerTradeTransaction::partner() const -> CCharEntity*
{
    return partner_;
}

auto PlayerTradeTransaction::abort(CCharEntity* leaving) -> void
{
    CCharEntity* other = initiator_ == leaving ? partner_ : initiator_;

    // Wake the counterparty's client out of the trade UI — their
    // tradePending still points at `leaving`. Without this they're
    // stuck in trade until relog.
    other->pushPacket<GP_SERV_COMMAND_ITEM_TRADE_RES>(leaving, GP_ITEM_TRADE_RES_KIND::Cancell);
    other->tradePending.clean();
    leaving->tradePending.clean();

    // Must be last — removeTransaction destroys *this via the unique_ptr dtor.
    initiator_->removeTransaction(this);
}

auto PlayerTradeTransaction::setAcceptedBy(const CCharEntity* who) -> void
{
    if (who == initiator_)
    {
        initiatorAccepted_ = true;
    }
    else if (who == partner_)
    {
        partnerAccepted_ = true;
    }
}

auto PlayerTradeTransaction::isAcceptedBy(const CCharEntity* who) const -> bool
{
    if (who == initiator_)
    {
        return initiatorAccepted_;
    }
    if (who == partner_)
    {
        return partnerAccepted_;
    }
    return false;
}

auto PlayerTradeTransaction::bothAccepted() const -> bool
{
    return initiatorAccepted_ && partnerAccepted_;
}

auto PlayerTradeTransaction::clearBothAcceptances() -> void
{
    initiatorAccepted_ = false;
    partnerAccepted_   = false;
}

auto PlayerTradeTransaction::commitAndClose() -> bool
{
    const bool ok      = commit();
    const auto resKind = ok ? GP_ITEM_TRADE_RES_KIND::End : GP_ITEM_TRADE_RES_KIND::Cancell;

    initiator_->pushPacket<GP_SERV_COMMAND_ITEM_TRADE_RES>(partner_, resKind);
    initiator_->tradePending.clean();
    partner_->pushPacket<GP_SERV_COMMAND_ITEM_TRADE_RES>(initiator_, resKind);
    partner_->tradePending.clean();

    // Must be last — removeTransaction destroys *this.
    initiator_->removeTransaction(this);
    return ok;
}

auto PlayerTradeTransaction::slotsFor(const CCharEntity* who) -> std::array<Slot, kMaxSlots>*
{
    if (who == initiator_)
    {
        return &initiatorSlots_;
    }
    if (who == partner_)
    {
        return &partnerSlots_;
    }
    return nullptr;
}

auto PlayerTradeTransaction::slotsFor(const CCharEntity* who) const -> const std::array<Slot, kMaxSlots>*
{
    if (who == initiator_)
    {
        return &initiatorSlots_;
    }
    if (who == partner_)
    {
        return &partnerSlots_;
    }
    return nullptr;
}

auto PlayerTradeTransaction::takeSlot(CCharEntity* who, std::uint8_t txSlot, std::uint8_t invSlot, std::uint32_t qty) -> bool
{
    if (txSlot >= kMaxSlots || qty == 0 || who == nullptr)
    {
        return false;
    }
    auto* slots = slotsFor(who);
    if (slots == nullptr)
    {
        return false;
    }
    // Returning an already-taken slot first — the caller may be
    // replacing a qty or swapping items without a separate releaseSlot.
    if ((*slots)[txSlot].item != nullptr)
    {
        releaseSlot(who, txSlot);
    }

    auto* container = who->getStorage(LOC_INVENTORY);
    if (container == nullptr)
    {
        return false;
    }
    auto* item = container->GetItem(invSlot);
    if (item == nullptr || qty > item->getQuantity())
    {
        return false;
    }

    auto prior = ItemStore::moveToTransaction(item, this);
    if (!prior)
    {
        return false;
    }
    (*slots)[txSlot] = Slot{ item, invSlot, qty, std::move(*prior) };
    return true;
}

auto PlayerTradeTransaction::holds(const CItem* item) const -> bool
{
    if (!isOpen() || item == nullptr)
    {
        return false;
    }
    for (const auto& slot : initiatorSlots_)
    {
        if (slot.item == item)
        {
            return true;
        }
    }
    for (const auto& slot : partnerSlots_)
    {
        if (slot.item == item)
        {
            return true;
        }
    }
    return false;
}

auto PlayerTradeTransaction::doRollback() -> void
{
    const auto restoreSide = [](std::array<Slot, kMaxSlots>& slots)
    {
        for (auto& slot : slots)
        {
            if (slot.item != nullptr)
            {
                restoreEntry(slot.item, slot.priorOwner);
                slot.item = nullptr;
            }
        }
    };
    restoreSide(initiatorSlots_);
    restoreSide(partnerSlots_);
}

auto PlayerTradeTransaction::releaseSlot(const CCharEntity* who, const std::uint8_t txSlot) -> void
{
    if (txSlot >= kMaxSlots || who == nullptr)
    {
        return;
    }
    auto* slots = slotsFor(who);
    if (slots == nullptr)
    {
        return;
    }
    auto& slot = (*slots)[txSlot];
    if (slot.item == nullptr)
    {
        return;
    }
    ItemStore::deliverToInventory(slot.item, who->id, LOC_INVENTORY, slot.invSlot);
    slot = Slot{};
}

auto PlayerTradeTransaction::itemAt(const CCharEntity* who, const std::uint8_t txSlot) const -> CItem*
{
    const auto* slots = slotsFor(who);
    if (!isOpen() || slots == nullptr || txSlot >= kMaxSlots)
    {
        return nullptr;
    }
    return (*slots)[txSlot].item;
}

auto PlayerTradeTransaction::offeredQtyAt(const CCharEntity* who, const std::uint8_t txSlot) const -> std::uint32_t
{
    const auto* slots = slotsFor(who);
    if (!isOpen() || slots == nullptr || txSlot >= kMaxSlots)
    {
        return 0;
    }
    return (*slots)[txSlot].offeredQty;
}

namespace
{
// Count occupied slots in a side's array.
auto countItems(const std::array<PlayerTradeTransaction::Slot, PlayerTradeTransaction::kMaxSlots>& slots) -> std::size_t
{
    std::size_t n = 0;
    for (const auto& s : slots)
    {
        if (s.item != nullptr)
        {
            ++n;
        }
    }
    return n;
}

// Pre-validate that `receiver` can accept `sender`'s offer: enough
// free inventory slots + no rare-dup conflicts.
auto canReceive(const std::array<PlayerTradeTransaction::Slot, PlayerTradeTransaction::kMaxSlots>& senderSlots, CCharEntity* receiver) -> bool
{
    if (receiver == nullptr)
    {
        return false;
    }
    const std::size_t offerCount = countItems(senderSlots);
    if (offerCount == 0)
    {
        return true;
    }
    if (receiver->getStorage(LOC_INVENTORY)->GetFreeSlotsCount() < offerCount)
    {
        return false;
    }
    for (const auto& slot : senderSlots)
    {
        if (slot.item != nullptr && slot.item->hasFlag(ItemFlag::Rare))
        {
            if (charutils::HasItem(receiver, slot.item->getID()))
            {
                return false;
            }
        }
    }
    return true;
}
} // namespace

auto PlayerTradeTransaction::doCommit() -> bool
{
    if (!bothAccepted() || initiator_ == nullptr || partner_ == nullptr)
    {
        return false;
    }

    // Pre-validation: both destinations must have slot room + no
    // rare-dup collisions. First gate before any mutation.
    if (!canReceive(initiatorSlots_, partner_) ||
        !canReceive(partnerSlots_, initiator_))
    {
        return false;
    }

    // True two-phase commit:
    //   Phase A — deliver clones to both receivers. Track every
    //             AddItem so we can undo on partial failure.
    //   Phase B — consume originals from senders. Only runs once
    //             every Phase A delivery succeeded.
    //
    // If Phase A fails partway (rare-dup race past canReceive,
    // inventory-full race, DB error), we undo the already-completed
    // AddItems via UpdateItem(-qty) and return false. No original
    // has been consumed at that point, so the subsequent tx rollback
    // restores sender state cleanly. No dup window.

    struct Delivery
    {
        CCharEntity*  receiver{};
        std::uint8_t  invSlot{};
        std::uint32_t qty{};
    };
    std::vector<Delivery> delivered;
    delivered.reserve(kMaxSlots * 2);

    const auto deliverSide = [&](const std::array<Slot, kMaxSlots>& senderSlots, CCharEntity* receiver) -> bool
    {
        for (const auto& slot : senderSlots)
        {
            if (slot.item == nullptr)
            {
                continue;
            }
            const std::uint32_t qty = slot.offeredQty;

            std::uint8_t deliveredSlot = ERROR_SLOTID;
            if (slot.item->getStackSize() == 1 && qty == 1)
            {
                auto clone = itemutils::GetItem(slot.item);
                if (clone == nullptr)
                {
                    return false;
                }
                deliveredSlot = charutils::AddItem(receiver, LOC_INVENTORY, std::move(clone));
            }
            else
            {
                deliveredSlot = charutils::AddItem(receiver, LOC_INVENTORY, slot.item->getID(), qty);
            }
            if (deliveredSlot == ERROR_SLOTID)
            {
                return false;
            }
            delivered.push_back({ receiver, deliveredSlot, qty });
        }
        return true;
    };

    if (!deliverSide(initiatorSlots_, partner_) ||
        !deliverSide(partnerSlots_, initiator_))
    {
        // Phase A failed. Undo every successful AddItem — no sender
        // has been consumed yet, so tx rollback will restore custody
        // to the sender's inventory slot.
        for (const auto& d : delivered)
        {
            charutils::UpdateItem(d.receiver, LOC_INVENTORY, d.invSlot, -static_cast<std::int32_t>(d.qty));
        }
        return false;
    }

    // Phase B — commit point. Every delivery succeeded; senders lose
    // their offered items atomically. consume() drops the CItem if
    // the stack hits zero; we null the slot to avoid rollback touching
    // freed memory.
    const auto consumeSide = [](std::array<Slot, kMaxSlots>& senderSlots, CCharEntity* sender)
    {
        for (auto& slot : senderSlots)
        {
            if (slot.item == nullptr)
            {
                continue;
            }
            ItemStore::deliverToInventory(slot.item, sender->id, LOC_INVENTORY, slot.invSlot);
            ItemStore::consume(sender, LOC_INVENTORY, slot.invSlot, slot.offeredQty);
            slot = Slot{};
        }
    };
    consumeSide(initiatorSlots_, initiator_);
    consumeSide(partnerSlots_, partner_);
    return true;
}
