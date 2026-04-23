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

#include "transaction.h"

#include "common/logging.h"

#include "items/item_store.h"

#include <atomic>

namespace
{
auto allocTxId() -> uint64_t
{
    static std::atomic<uint64_t> s_counter{ 1 };
    return s_counter.fetch_add(1, std::memory_order_relaxed);
}
} // namespace

Transaction::Transaction()
: id_(allocTxId())
{
}

Transaction::~Transaction() = default;

auto Transaction::id() const -> uint64_t
{
    return id_;
}

auto Transaction::isOpen() const -> bool
{
    return state_ == State::Open;
}

auto Transaction::commit() -> bool
{
    if (state_ != State::Open)
    {
        return false;
    }
    if (!doCommit())
    {
        // Precondition failure — stay Open so caller can retry/rollback.
        return false;
    }
    state_ = State::Committed;
    return true;
}

auto Transaction::rollback() -> void
{
    if (state_ != State::Open)
    {
        return;
    }
    doRollback();
    state_ = State::RolledBack;
}

auto Transaction::restoreEntry(CItem* item, const ItemOwner& priorOwner) -> void
{
    if (item != nullptr)
    {
        ItemStore::restoreFromTransaction(item, priorOwner, xi::Badge<Transaction>{});
    }
}

auto Transaction::silentRollbackIfOpen() -> void
{
    if (state_ == State::Open)
    {
        rollback();
    }
}
