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

// result.distortion
// For general attacks, this value is used to determine the model hit distortion animation that will play from the attack.
// Normal hits will generally use 0, 1, or 2 while critical hits will generally use 2 or 3.
// 2 bits (lower bits of former scale field)
enum class HitDistortion : uint8_t
{
    None   = 0, // 00 - 0.0 distortion
    Light  = 1, // 01 - 0.25 distortion
    Medium = 2, // 10 - 0.5 distortion
    Heavy  = 3, // 11 - 1.0 distortion (critical hits)
};
