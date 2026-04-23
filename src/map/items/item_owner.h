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

#include <cstdint>
#include <variant>

// ItemOwner — where an item currently lives. Exactly one variant is
// active at any moment. See docs/design/item-ownership-model.md §3.2.
//
// Three variants cover every meaningful non-terminal state. Destruction
// isn't a variant; when an item is destroyed, its CItem* is freed by
// ItemStore::destroy and no longer exists to observe. A "Consumed" marker
// on a soon-to-be-freed object would be dead weight.
//
// "In the world" / "on the ground" isn't modeled either — FFXI has no
// such mechanic. Treasure pools (which might feel like it) are a
// Transaction subclass, so items in them have owner InTransaction.

namespace xi::item
{
// Home: item lives in one of a character's containers — inventory,
// mog safe, wardrobe, storage, temp items, etc. The `location` field
// carries the specific LOC_* container id; this variant covers the
// whole family.
struct InCharContainer
{
    uint32_t charId{};
    uint8_t  location{}; // LOC_INVENTORY, LOC_MOGSAFE, LOC_WARDROBE[1-8], LOC_STORAGE, etc.
    uint8_t  slot{};
};

// In flight: owned by a live Transaction object. Tag-only — the actual
// tx handle is looked up via CCharEntity::activeTransaction<T>().
struct InTransaction
{
};

// Pre-registration: a freshly-constructed CItem that hasn't been
// placed anywhere yet. ItemStore::place* promotes it.
struct Unowned
{
};
} // namespace xi::item

using ItemOwner = std::variant<
    xi::item::Unowned,
    xi::item::InCharContainer,
    xi::item::InTransaction>;
