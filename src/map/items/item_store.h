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

// ItemStore — single-writer authority for item ownership AND lifetime.
//
// Every CItem is created via ItemStore::create<T>(...), which returns
// a `std::unique_ptr<T>`. Lifetime is managed by the unique_ptr chain —
// raw `new CItem*` / `delete PItem` are banned outside this file.
// Destruction happens when the unique_ptr goes out of scope (ItemStore's
// private drop is the only call site that raw-deletes).
//
// Every transition of an item's ItemOwner goes through ItemStore.
// The Badge<ItemStore> gate on CItem::setOwner enforces this at
// compile time. See docs/design/item-ownership-model.md §3.
//
// Reads (CItem::owner()) are free; writes route here.

class ItemStore
{
public:
    // ---- Factory ------------------------------------------------------
    //
    // Allocate a fresh item of subtype T. Starts Unowned — caller
    // promotes with placeInInventory or moveToTransaction, or passes it through
    // AddItem/InsertItem which take ownership by move.
    template <typename T, typename... Args>
    static auto create(Args&&... args) -> std::unique_ptr<T>
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

    // Clone an existing item. The clone starts Unowned even if the
    // source wasn't.
    template <typename T>
    static auto clone(const T& source) -> std::unique_ptr<T>
    {
        auto item = std::unique_ptr<T>(new T(source));
        clearOwner(item.get());
        return item;
    }

    // Drop an item that a character may have a tx holding. If such a
    // tx exists (from the character's transactions), removes it
    // first so we don't leave a dangling entry pointer. Use when a
    // container-level cleanup (UContainer close, etc.) is about to
    // destroy items that were stamped InTransaction.
    static auto dropOwnedBy(CCharEntity* character, const CItem* item) -> void;

    // ---- Queries ------------------------------------------------------

    static auto isInInventory(const CItem* item) -> bool;
    static auto isInTransaction(const CItem* item) -> bool;

    // ---- Home placement ----------------------------------------------

    // Promote an Unowned item to InCharContainer. Rejects items already in
    // a tx or another inventory (prior-state validated).
    static auto placeInInventory(CItem* item, uint32_t charId, uint8_t location, uint8_t slot) -> void;

    // Re-stamp an already-in-inventory item with new (location, slot).
    // Used by MoveItem / AddItemToRecycleBin where the owner stays the
    // same but its address inside the inventory changes.
    static auto reslotInventory(CItem* item, uint8_t newLocation, uint8_t newSlot) -> void;

    // Reset to Unowned. Only legal on InCharContainer / Unowned items
    // (a tx-owned item must be rolled back or committed first).
    static auto clearOwner(CItem* item) -> void;

    // ---- Tx primitives -----------------------------------------------
    //
    // moveToTransaction / adoptIntoTransaction return the prior owner
    // on success, or std::nullopt on rejection. The tx subclass stores
    // the prior owner in its own slot bookkeeping so rollback can
    // restore it.

    // Move item from InCharContainer into tx. Returns nullopt if the item
    // isn't InCharContainer (wrong prior state).
    static auto moveToTransaction(CItem* item, Transaction* tx) -> std::optional<ItemOwner>;

    // Adopt an Unowned item directly into a tx. Used by reconstruction
    // paths (items loaded from persistence tables — dbox, auction —
    // aren't in anyone's inventory; they go straight to the tx that
    // represents their table-backed state). Skips the ghost-InCharContainer
    // stamp that would make a save-path race persist them to
    // char_inventory. Prior owner is always Unowned on success.
    static auto adoptIntoTransaction(CItem* item, Transaction* tx) -> std::optional<ItemOwner>;

    // Tx hands an item to an inventory slot. Used by commit paths
    // that deliver an item to a character (trade swap, dbox retrieve,
    // auction delivery). Requires the item was InTransaction.
    static auto deliverToInventory(CItem* item, uint32_t charId, uint8_t location, uint8_t slot) -> bool;

    // Called by Transaction::rollback (through subclass doRollback) to
    // restore an item to its recorded prior owner. Badge-gated — only
    // Transaction can call.
    static auto restoreFromTransaction(CItem* item, ItemOwner priorOwner, xi::Badge<Transaction>) -> void;

    // True if the item is in a tx or pinned by a binding. Callers
    // that also care about bazaar listing add
    // `|| PItem->getCharPrice() > 0` themselves.
    static auto isBusy(const CItem* item) -> bool;

    // Decrement `qty` from an inventory slot. Drops the CItem when
    // the stack hits zero. Item must be InCharContainer at call time
    // (tx callers should deliverToInventory first). Returns the
    // consumed item id (0 on failure).
    static auto consume(CCharEntity* owner, std::uint8_t location, std::uint8_t slot, std::uint32_t qty) -> std::uint16_t;

private:
    static auto writeOwner(CItem* item, ItemOwner newOwner) -> void;

    // Internal delete path. Public callers manage lifetimes through
    // unique_ptr / container ownership — raw delete is banned.
    static auto destroy(const CItem* item) -> void;
};
