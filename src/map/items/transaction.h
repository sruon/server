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

#include "common/cbasetypes.h"
#include "common/types/badge.h"

#include "item_owner.h"

#include <cstdint>

class CItem;

// Transaction — base for every multi-step item interaction. Subclasses
// implement doCommit() / doRollback() / holds() over their own slot
// data; the public commit()/rollback() are non-virtual state-machine
// wrappers.
//
// Lifecycle contract: callers should commit() or rollback() before
// destruction. As a safety net, each subclass's dtor calls
// silentRollbackIfOpen() — this catches any path that dropped the
// tx without finalizing (zone change, disconnect, test teardown).
// Subclasses MUST declare their own dtor calling this helper, or
// Open state will silently leak items stamped InTransaction.
//
// See docs/design/item-ownership-model.md §3.3 / §4.

class Transaction
{
public:
    enum class State : uint8_t
    {
        Open,
        Committed,
        RolledBack,
    };

    Transaction();
    virtual ~Transaction();

    Transaction(const Transaction&)                    = delete;
    auto operator=(const Transaction&) -> Transaction& = delete;
    Transaction(Transaction&&)                         = delete;
    auto operator=(Transaction&&) -> Transaction&      = delete;

    // Monotonic id assigned at construction. Used by PendingDeclaredTrade
    // to compare "tx we pended against" vs "tx active now" without
    // relying on pointer identity (allocator reuse is real). uint64 to
    // avoid wraparound collision on long-lived servers.
    auto id() const -> uint64_t;
    auto isOpen() const -> bool;

    // True if this tx holds the given item in its slots (Open txs only).
    virtual auto holds(const CItem* item) const -> bool = 0;

    // Finalize. Returns false if state isn't Open or doCommit() rejected
    // (precondition failure — tx stays Open so caller can roll back).
    auto commit() -> bool;

    // Idempotent against terminal states.
    auto rollback() -> void;

protected:
    virtual auto doCommit() -> bool = 0;

    // Subclass rollback: iterate slots, call restoreEntry on each held
    // item. Pure virtual — every subclass must implement, even if the
    // body is `{}` for custody-less txs.
    virtual auto doRollback() -> void = 0;

    // Helper for subclass doRollback() — wraps the Badge-gated
    // ItemStore::restoreFromTransaction so subclasses don't need access to
    // Badge<Transaction>.
    static auto restoreEntry(CItem* item, const ItemOwner& priorOwner) -> void;

    // Subclass dtors call this as their first statement. Virtual
    // dispatch on doRollback works here because Derived is still live
    // (we're running inside ~Derived). Silent — no abort — so stray
    // destruction paths (zone change, test teardown) don't blow up.
    // A forgotten subclass dtor means items stamped InTransaction
    // leak; detectable downstream via ItemStore::destroy's abort.
    auto silentRollbackIfOpen() -> void;

private:
    State    state_{ State::Open };
    uint64_t id_{};
};
