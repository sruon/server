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

// Side pointers pinning inventory items for a purpose: equipped,
// placed as furniture, listed in bazaar. Items stay in place — custody
// is not transferred. Bound items are rejected from tx custody.

namespace xi::bindings
{
enum class Reason : uint8
{
    Furniture,
    Bazaar,
};

struct EquipBinding
{
    uint8  location{};
    uint8  invSlot{};
    CItem* item{};
};

struct Binding
{
    Reason reason{};
    uint8  location{};
    uint8  invSlot{};
    CItem* item{};
    uint8  layoutSlot{}; // furniture only
};

constexpr std::size_t kEquipBindingCount = 18;
} // namespace xi::bindings

class CharacterInventoryBindings
{
public:
    auto equipBinding(uint8 slot) const -> const std::optional<xi::bindings::EquipBinding>&;
    auto bindings() const -> const std::vector<xi::bindings::Binding>&;

    auto isBoundItem(const CItem* item) const -> bool;

    auto bindEquip(uint8 equipSlot, uint8 location, uint8 invSlot, CItem* item) -> bool;
    auto clearEquip(uint8 equipSlot) -> void;

    auto addFurniture(uint8 location, uint8 invSlot, CItem* item, uint8 layoutSlot) -> bool;
    auto removeFurniture(uint8 location, uint8 invSlot) -> void;

    auto addBazaar(uint8 location, uint8 invSlot, CItem* item) -> bool;
    auto removeBazaar(uint8 location, uint8 invSlot) -> void;

    // Called when a bound item moves slots (/put). Bindings follow.
    auto updateItemLocation(uint8 oldLocation, uint8 oldSlot, uint8 newLocation, uint8 newSlot) -> void;

private:
    auto addBinding(xi::bindings::Binding binding) -> bool;
    auto removeBinding(xi::bindings::Reason reason, uint8 location, uint8 invSlot) -> void;

    std::array<std::optional<xi::bindings::EquipBinding>, xi::bindings::kEquipBindingCount> equip_{};
    std::vector<xi::bindings::Binding>                                                      bindings_{};
};
