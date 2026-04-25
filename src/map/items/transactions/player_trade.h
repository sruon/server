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

#include <array>
#include <cstdint>

class CCharEntity;
class CItem;

// Custody for a player-to-player trade. Owned by the initiator;
// partner reaches it via tradePending → activePlayerTradeTx.
//
// Each side's items enter custody on 0x034 trade_list via takeSlot.
// Commit is an atomic two-sided swap: both inventories pre-validated,
// then items handed across. Any failure rolls both halves back.

class PlayerTradeTransaction : public Transaction
{
public:
    static constexpr std::size_t kMaxSlots = 9;

    // Installs on `initiator`. Returns nullptr if either side already
    // has an active PlayerTradeTransaction.
    static auto start(CCharEntity* initiator, CCharEntity* partner) -> PlayerTradeTransaction*;

    auto initiator() const -> CCharEntity*;
    auto partner() const -> CCharEntity*;

    // Cancel the trade for `leaving`. Notifies the counterparty, clears
    // tradePending both sides, removes the tx. Destroys *this.
    auto abort(CCharEntity* leaving) -> void;

    auto setAcceptedBy(const CCharEntity* who) -> void;
    auto isAcceptedBy(const CCharEntity* who) const -> bool;
    auto bothAccepted() const -> bool;

    // Any offer change (takeSlot/releaseSlot) must invalidate prior
    // acceptances; the 0x034 handler calls this before applying the op.
    auto clearBothAcceptances() -> void;

    // Commit + notify both clients + clean up. Destroys *this.
    auto commitAndClose() -> bool;

    auto takeSlot(CCharEntity* who, std::uint8_t txSlot, std::uint8_t invSlot, std::uint32_t qty) -> bool;
    auto releaseSlot(const CCharEntity* who, std::uint8_t txSlot) -> void;

    auto itemAt(const CCharEntity* who, std::uint8_t txSlot) const -> CItem*;
    auto offeredQtyAt(const CCharEntity* who, std::uint8_t txSlot) const -> std::uint32_t;

    struct Slot
    {
        CItem*        item{};
        std::uint8_t  invSlot{ 0xFF };
        std::uint32_t offeredQty{};
        ItemOwner     priorOwner{};
    };

    PlayerTradeTransaction(xi::Badge<PlayerTradeTransaction>, CCharEntity* initiator, CCharEntity* partner);

    ~PlayerTradeTransaction() override
    {
        silentRollbackIfOpen();
    }

    auto holds(const CItem* item) const -> bool override;

protected:
    auto doCommit() -> bool override;
    auto doRollback() -> void override;

private:
    auto slotsFor(const CCharEntity* who) -> std::array<Slot, kMaxSlots>*;
    auto slotsFor(const CCharEntity* who) const -> const std::array<Slot, kMaxSlots>*;

    CCharEntity* initiator_{};
    CCharEntity* partner_{};
    bool         initiatorAccepted_{};
    bool         partnerAccepted_{};

    std::array<Slot, kMaxSlots> initiatorSlots_{};
    std::array<Slot, kMaxSlots> partnerSlots_{};
};
