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

#pragma once

#include "common/types/badge.h"
#include "items/item_owner.h"
#include "items/transaction.h"

#include <sol/sol.hpp>

#include <array>
#include <cstdint>

class CBaseEntity;
class CCharEntity;
class CItem;

// Custody for an NPC trade's offer phase. Opens on 0x036 item_transfer;
// survives across event callbacks so rewards and commit sequence with
// the event's end. On commit, reserved slots decrement consume-qty
// (possibly destroying the CItem); unreserved items return to the player.

class NpcTradeTransaction : public Transaction
{
public:
    static constexpr std::size_t kMaxSlots = 8;

    static auto start(CCharEntity* player, CBaseEntity* npc) -> NpcTradeTransaction*;

    // Move an inventory item into the tx at `txSlot`. `qty` is the
    // consume amount (can be less than the stack size).
    auto takeSlot(std::uint8_t txSlot, std::uint8_t invSlot, std::uint32_t qty) -> bool;

    // Record offered gil. Slot 0 of the trade protocol is gil. Gil is
    // intent-only — not moved into custody; deducted on commit if
    // reserved.
    auto offerGil(std::uint32_t amount) -> bool;
    auto reserveGil(std::uint32_t amount) -> bool;
    auto clearGilReservation() -> void;

    // Find the first unreserved slot offering at least `qty` of
    // `itemId` and mark it reserved.
    auto reserveFromOffer(std::uint16_t itemId, std::uint32_t qty) -> bool;
    auto clearReservation(std::uint8_t txSlot) -> void;

    // Commit + push ITEM_SAME + removeTransaction. Destroys *this.
    auto commitAndClose() -> bool;

    // Accessors used by the declarative-trade runner. Return defaults
    // when the tx is no longer Open.
    auto itemAt(std::uint8_t txSlot) const -> CItem*;
    auto offeredQtyAt(std::uint8_t txSlot) const -> std::uint32_t;
    auto reservedQtyAt(std::uint8_t txSlot) const -> std::uint32_t;
    auto offeredGil() const -> std::uint32_t;
    auto reservedGil() const -> std::uint32_t;

    auto player() const -> CCharEntity*
    {
        return player_;
    }

    // Arm an event-finish commit hook. OnEventFinish for `eventId` will
    // invoke `onFinish(player, option, npc, confirmations)` and commit.
    auto armPending(uint32_t eventId, CBaseEntity* npc, sol::protected_function onFinish) -> void;

    // Non-zero when a pending event-finish hook is armed.
    auto pendingEventId() const -> uint32_t;
    auto pendingNpc() const -> CBaseEntity*;
    auto pendingOnFinish() const -> const sol::protected_function&;

    NpcTradeTransaction(xi::Badge<NpcTradeTransaction>, CCharEntity* player, CBaseEntity* npc);

    ~NpcTradeTransaction() override
    {
        silentRollbackIfOpen();
    }

    auto holds(const CItem* item) const -> bool override;

protected:
    auto doCommit() -> bool override;
    auto doRollback() -> void override;

private:
    struct Slot
    {
        CItem*        item{};
        std::uint8_t  invSlot{ 0xFF };
        std::uint32_t offeredQty{};
        std::uint32_t reservedQty{};
        ItemOwner     priorOwner{};
    };

    CCharEntity*                player_{};
    CBaseEntity*                npc_{};
    std::array<Slot, kMaxSlots> slots_{};
    std::uint32_t               offeredGil_{};
    std::uint32_t               reservedGil_{};

    // Deferred commit: set by the declarative dispatcher when a trade
    // starts an event; consumed in OnEventFinish.
    uint32_t                pendingEventId_{};
    CBaseEntity*            pendingNpc_{};
    sol::protected_function pendingOnFinish_;
};
