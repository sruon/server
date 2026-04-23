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

class CCharEntity;

// TransactionHandle<T> — non-owning reference to an installed transaction.
//
// Only CCharEntity::addTransaction can construct a non-empty TransactionHandle
// (Badge<CCharEntity> on the ctor). Tx factories that install on
// CCharEntity::transactions declare `start(...) -> TransactionHandle<T>`
// and must therefore route through addTransaction to return a value —
// forgetting the install step is a compile error, not a runtime abort.
//
// TransactionHandle is trivially copyable and acts like T* for access
// (operator->, get(), boolean conversion). Lifetime of the pointee is
// owned by CCharEntity::transactions; the handle does not extend it.
//
// Exception: ItemUseTransaction::start returns std::unique_ptr<T>
// because its tx is owned by CItemState::tx_ for the duration of an
// item-use action, not by CCharEntity::transactions. Documented on
// item_use.h:start.
//
// activeTransaction<T>() still returns raw T* — it's a lookup on an already-
// installed tx, not a construction path, so the TransactionHandle gate doesn't
// apply there.

template <typename T>
class TransactionHandle
{
public:
    TransactionHandle() = default;

    TransactionHandle(T* p, xi::Badge<CCharEntity>)
    : ptr_(p)
    {
    }

    auto get() const -> T*
    {
        return ptr_;
    }
    auto operator->() const -> T*
    {
        return ptr_;
    }
    auto operator*() const -> T&
    {
        return *ptr_;
    }
    explicit operator bool() const
    {
        return ptr_ != nullptr;
    }

    auto operator==(std::nullptr_t) const -> bool
    {
        return ptr_ == nullptr;
    }
    auto operator!=(std::nullptr_t) const -> bool
    {
        return ptr_ != nullptr;
    }

private:
    T* ptr_{};
};
