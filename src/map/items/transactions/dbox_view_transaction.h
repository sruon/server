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

#include "items/item_owner.h"
#include "items/transaction.h"

class CCharEntity;
class CItem;

// Custody stamp for an item loaded into the delivery-box view while
// the mailbox window is open. Legacy dboxutils still drives DB rows and
// retrieval; this tx exists only to make ItemStore::isBusy return true
// on loaded items.

class DboxViewTransaction : public Transaction
{
public:
    static auto start(CCharEntity* owner, CItem* item) -> DboxViewTransaction*;

    ~DboxViewTransaction() override
    {
        silentRollbackIfOpen();
    }

    auto holds(const CItem* item) const -> bool override;

protected:
    auto doCommit() -> bool override;
    auto doRollback() -> void override;

private:
    DboxViewTransaction() = default;

    CItem*    item_{};
    ItemOwner priorOwner_{};
};
