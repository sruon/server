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

// Base for multi-step item interactions. Subclasses implement the three
// virtuals; commit() / rollback() are non-virtual state-machine wrappers.
//
// Each subclass dtor must call silentRollbackIfOpen() to catch paths
// that drop the tx without finalizing (disconnect, zone change, test
// teardown). Forgetting it leaks items stamped InTransaction.

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

    // Monotonic id. Used to distinguish "the tx we pended against" from
    // a later tx that reused the same address.
    auto id() const -> uint64_t;
    auto isOpen() const -> bool;

    virtual auto holds(const CItem* item) const -> bool = 0;

    // Runs doCommit(). Returns false if not Open or doCommit rejected.
    auto commit() -> bool;

    // Idempotent on terminal states.
    auto rollback() -> void;

protected:
    virtual auto doCommit() -> bool   = 0;
    virtual auto doRollback() -> void = 0;

    // Helper for doRollback(). Wraps the Badge-gated ItemStore restore.
    static auto restoreEntry(CItem* item, const ItemOwner& priorOwner) -> void;

    // Call from the subclass dtor (first statement). Virtual dispatch on
    // doRollback works because Derived is still alive inside ~Derived.
    auto silentRollbackIfOpen() -> void;

private:
    State    state_{ State::Open };
    uint64_t id_{};
};
