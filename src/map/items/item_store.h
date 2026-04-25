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
#include <memory>
#include <optional>
#include <utility>

class CCharEntity;
class CItem;
class Transaction;

// Single writer for CItem ownership and lifetime. Items are created
// here, ownership transitions route here, and the only raw delete
// lives in destroy().

class ItemStore
{
public:
    template <typename T, typename... Args>
    static auto create(Args&&... args) -> std::unique_ptr<T>
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

    template <typename T>
    static auto clone(const T& source) -> std::unique_ptr<T>
    {
        auto item = std::unique_ptr<T>(new T(source));
        clearOwner(item.get());
        return item;
    }

    // Retire any tx holding `item` on `character`, then destroy it.
    static auto dropOwnedBy(CCharEntity* character, const CItem* item) -> void;

    static auto isInInventory(const CItem* item) -> bool;
    static auto isInTransaction(const CItem* item) -> bool;

    // Unowned → InCharContainer. Rejects items already in a tx or inventory.
    static auto placeInInventory(CItem* item, uint32_t charId, uint8_t location, uint8_t slot) -> void;

    // Update (location, slot) on an already-inventory-owned item.
    static auto reslotInventory(CItem* item, uint8_t newLocation, uint8_t newSlot) -> void;

    // Reset to Unowned. Not valid on tx-held items.
    static auto clearOwner(CItem* item) -> void;

    // InCharContainer → InTransaction. Returns the prior owner for rollback.
    static auto moveToTransaction(CItem* item, Transaction* tx) -> std::optional<ItemOwner>;

    // Unowned → InTransaction. For items rebuilt from persistence (dbox,
    // auction) that are never in a character container.
    static auto adoptIntoTransaction(CItem* item, Transaction* tx) -> std::optional<ItemOwner>;

    // InTransaction → InCharContainer on commit paths.
    static auto deliverToInventory(CItem* item, uint32_t charId, uint8_t location, uint8_t slot) -> bool;

    // Called from Transaction::rollback. Restores the owner recorded at reserve time.
    static auto restoreFromTransaction(CItem* item, ItemOwner priorOwner, xi::Badge<Transaction>) -> void;

    // In a tx, or pinned by any binding. Caller supplies the owning
    // character so bindings can be queried; pass nullptr for the char
    // when you only need the in-tx check.
    static auto isBusy(const CCharEntity* owner, const CItem* item) -> bool;

    // Drop qty from an inventory slot. Destroys the CItem if the stack hits zero.
    static auto consume(CCharEntity* owner, std::uint8_t location, std::uint8_t slot, std::uint32_t qty) -> std::uint16_t;

private:
    static auto writeOwner(CItem* item, ItemOwner newOwner) -> void;
    static auto destroy(const CItem* item) -> void;
};
