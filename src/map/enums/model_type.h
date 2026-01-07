/*
===========================================================================

  Copyright (c) 2025 LandSandBoat Dev Teams

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

// Model type determines SubKind for NPC entities (look.size field)
// Maps directly to packet 0x00E SubKind values
enum class ModelType : uint8
{
    Standard  = 0,
    Equipped  = 1,
    Door      = 2,
    Elevator  = 3,
    Ship      = 4,
    Unk5      = 5,
    Automaton = 6,
    Chocobo   = 7,
};
