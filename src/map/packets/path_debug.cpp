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

#include "path_debug.h"

#include "data/enums/entity_flags.h"
#include "entity_update.h"

auto PathDebugMarker::operator==(const PathDebugMarker& other) const -> bool
{
    return position.x == other.position.x && position.y == other.position.y && position.z == other.position.z &&
           model == other.model && name == other.name;
}

CPathDebugPacket::CPathDebugPacket(uint32 id, uint16 targid, const PathDebugMarker* marker)
{
    setType(0x0E);
    setSize(0x48);
    ref<uint32>(0x04) = id;
    ref<uint16>(0x08) = targid;
    if (!marker)
    {
        ref<uint8>(0x0A) = 0x30;
        ref<uint8>(0x1F) = 0x02;
        return;
    }

    ref<uint8>(0x0A)  = 0x1F; // Position, claim, general, name and model; also valid for a resend.
    ref<float>(0x0C)  = marker->position.x;
    ref<float>(0x10)  = marker->position.y - 1.0f; // Lift the marker/name one unit; negative Y is up. Keep the path coordinates unchanged.
    ref<float>(0x14)  = marker->position.z;
    ref<uint32>(0x18) = 0x00008000; // GroundFlag: preserve the sent height instead of snapping to terrain, as with ships.
    // No movement time or speed: initialize the position directly on each update.
    ref<uint32>(0x20) = 0x00000100; // CliPosInitFlag
    ref<uint32>(0x21) |= static_cast<uint32>(xi::EntityFlags::Untargetable | xi::EntityFlags::HideHp);
    ref<uint8>(0x28)  = 0x20; // Priority render; leave the NPC interaction flag unset.
    ref<uint8>(0x2B)  = 0x10; // No entity collision; keep the name visible.
    ref<uint16>(0x30) = MODEL_STANDARD;
    ref<uint16>(0x32) = marker->model;
    std::memcpy(buffer_.data() + 0x34, marker->name.data(), std::min<std::size_t>(marker->name.size(), PacketNameLength - 1));
}
