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

#include <cstdint>

// result.info
// RUN abilities rune element/mixing
// 5 bits
enum class RuneElement : uint8_t
{
    ElementalMix = 0, // 00000 - If the runes are different elemental types, then mixing will occur and animation id 0 is used.
    Ignis        = 1, // 00001
    Gelus        = 2, // 00010
    Flabra       = 3, // 00011
    Tellus       = 4, // 00100
    Suplor       = 5, // 00101
    Unda         = 6, // 00110
    Lux          = 7, // 00111
    Tenebrae     = 8, // 01000
};
