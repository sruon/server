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

#include "basic.h"
#include "common/mmo.h"

#include <string>

struct PathDebugMarker
{
    static constexpr uint16 DefaultModel = 1525; // Warhorse Hoofprint

    position_t  position{};
    uint16      model{ DefaultModel };
    std::string name;

    auto operator==(const PathDebugMarker& other) const -> bool;
};

// A client-only, fixed-model NPC. A null marker serializes a despawn.
class CPathDebugPacket final : public CBasicPacket
{
public:
    CPathDebugPacket(uint32 id, uint16 targid, const PathDebugMarker* marker);
};
