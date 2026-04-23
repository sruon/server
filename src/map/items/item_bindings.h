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

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

class CItem;
class CCharEntity;

// Bindings — non-ownership references from a character to an
// inventory item. See docs/design/item-ownership-model.md §3.4.
//
// Unlike Transactions, a binding does NOT transfer custody. The
// item stays in its inventory slot; the binding is a side-channel
// pointer saying "this slot is in use for purpose X." Two kinds:
//   - Equip-slot bindings (18 slots — the standard gear slots,
//     including SLOT_LINK1 / SLOT_LINK2 for linkshells, which are
//     indices 16-17 in the equip array)
//   - Placed furniture (mog-house layout pins)
//
// Invariants:
//   B1  A binding points to an item whose owner is InCharContainer.
//   B2  Bound items can't be moved into a Transaction custody
//       without clearing the binding first — enforces retail's
//       "unequip before trading" rule.

namespace xi::bindings
{
struct EquipBinding
{
    uint8  location{}; // inventory container the item lives in
    uint8  invSlot{};  // slot index within that container
    CItem* item{};     // cached pointer; authoritative is (location, invSlot)
};

struct FurnitureBinding
{
    uint8  location{}; // mog storage container
    uint8  invSlot{};
    CItem* item{};
    uint8  layoutSlot{}; // where it's placed in the moghouse
};

constexpr std::size_t kEquipBindingCount = 18;
} // namespace xi::bindings

class CItemBindings
{
public:
    // ---- Queries (anyone) --------------------------------------------

    auto equipBinding(uint8 slot) const -> const std::optional<xi::bindings::EquipBinding>&;
    auto furnitureBindings() const -> const std::vector<xi::bindings::FurnitureBinding>&;

    // True if the given item is pinned by any binding.
    auto isBoundItem(const CItem* item) const -> bool;

    // ---- Equipment mutations (character-owned; no Badge — character
    //      pre-validates via its own equip/unequip flows) -----------

    auto bindEquip(uint8 equipSlot, uint8 location, uint8 invSlot, CItem* item) -> bool;
    auto clearEquip(uint8 equipSlot) -> void;

    // ---- Furniture mutations ------------------------------------------

    auto addFurniture(xi::bindings::FurnitureBinding binding) -> bool;
    auto removeFurniture(uint8 location, uint8 invSlot) -> void;

    // ---- Slot-move hook -----------------------------------------------

    // Called when a bound item moves inventory slots (e.g., /put).
    // Bindings follow the item atomically so B1 stays satisfied.
    auto updateItemLocation(uint8 oldLocation, uint8 oldSlot, uint8 newLocation, uint8 newSlot) -> void;

private:
    std::array<std::optional<xi::bindings::EquipBinding>, xi::bindings::kEquipBindingCount> equip_{};
    std::vector<xi::bindings::FurnitureBinding>                                             furniture_{};
};
