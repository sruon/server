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
#include <magic_enum/magic_enum.hpp>

// result.info
// 5 bits (bitflags)
// This is used for ranged attacks, weaponskills and melee attacks
enum class ActionInfo : uint8_t
{
    None        = 0,
    Defeated    = 1, // 00001 - Set when the action defeats the target
    CriticalHit = 2, // 00010 - Set when the action is a critical hit
};

template <>
struct magic_enum::customize::enum_range<ActionInfo>
{
    static constexpr bool is_flags = true;
};
using namespace magic_enum::bitwise_operators;
