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
#include "items/transaction.h"
#include "items/transaction_handle.h"

class CCharEntity;
class CItem;

// DboxViewTransaction — custody stamp for a package loaded from delivery_box.
//
// Purpose: while a player has their mailbox open, each loaded CItem
// needs an owner that makes `ItemStore::isBusy` return true. That's
// it. The tx is intentionally thin — legacy dboxutils still drives
// the DB rows, packet flow, and inventory hand-off on retrieve. The
// commit/rollback state machine runs but doesn't transfer items; the
// caller freeing the CItem (via ItemStore::dropOwnedBy on window
// close / retrieval) removes the tx first so there's no dangling
// reference.
//
// Intentionally mode-blind: dbox Send vs Recv lives on CDboxView
// (DboxMode); the tx's job is custody stamping only, so the two
// paths share this one class.

class DboxViewTransaction : public Transaction
{
public:
    // Install on `owner` and take adopt-custody of `item`. `item`
    // must be Unowned (freshly cloned via itemutils::GetItem). Rejects
    // if the character already has a tx holding `item`.
    static auto start(CCharEntity* owner, CItem* item) -> TransactionHandle<DboxViewTransaction>;

    // Badge-gated public ctor so make_unique can reach it.
    explicit DboxViewTransaction(xi::Badge<DboxViewTransaction>)
    {
    }

    ~DboxViewTransaction() override
    {
        silentRollbackIfOpen();
    }

    auto holds(const CItem* item) const -> bool override;

protected:
    // No-op: actual work is in legacy dboxutils. Return true so the
    // state machine transitions to Committed cleanly; the tx's job
    // (custody stamping) is done at open().
    auto doCommit() -> bool override;

    // Rollback is a no-op: the CItem's lifetime is driven by dboxView
    // / dropOwnedBy, not by this tx. On mailbox-close the item is
    // about to be freed anyway.
    auto doRollback() -> void override
    {
    }

private:
    CItem* item_{};
};
