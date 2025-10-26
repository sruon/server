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

// result.knockback
// 3 bits (upper bits of former scale field)
enum class Knockback : uint8_t
{
    None        = 0, // 000 - No knockback
    Minimal     = 1, // 001 - Vec: 0.083, Dumper: 0.075, Timer: 5.0
    Light       = 2, // 010 - Vec: 0.167, Dumper: 0.15, Timer: 5.0
    Moderate    = 3, // 011 - Vec: 0.167, Dumper: 0.15, Timer: 10.0
    Heavy       = 4, // 100 - Vec: 0.167, Dumper: 0.15, Timer: 18.0
    Severe      = 5, // 101 - Vec: 0.167, Dumper: 0.125, Timer: 30.0
    Extreme     = 6, // 110 - Vec: 0.167, Dumper: 0.1, Timer: 35.0
    Devastating = 7, // 111 - Vec: 0.167, Dumper: 0.05, Timer: 45.0
};
