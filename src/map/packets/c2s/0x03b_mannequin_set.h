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

#include "base.h"

enum class GP_CLI_COMMAND_MANNEQUIN_SET_CONTAINERINDEX : uint8_t
{
    Main   = 0x00,
    Sub    = 0x01,
    Ranged = 0x02,
    Head   = 0x03,
    Body   = 0x04,
    Hands  = 0x05,
    Legs   = 0x06,
    Feet   = 0x07,
};

enum class GP_CLI_COMMAND_MANNEQUIN_SET_KIND : uint32_t
{
    Equip      = 1,
    Unequip    = 2,
    UnequipAll = 5,
};

// https://github.com/atom0s/XiPackets/tree/main/world/client/0x003B
// This packet is sent by the client when interacting with a sub-container item. (ie. Mannequins)
GP_CLI_PACKET(GP_CLI_COMMAND_MANNEQUIN_SET,
              uint32_t Kind;           // PS2: (New; did not exist.)
              uint32_t Category1;      // PS2: (New; did not exist.)
              uint8_t  ItemIndex1;     // PS2: (New; did not exist.)
              uint8_t  ContainerIndex; // PS2: (New; did not exist.)
              uint16_t padding00;      // PS2: (New; did not exist.)
              uint32_t Category2;      // PS2: (New; did not exist.)
              uint8_t  ItemIndex2;     // PS2: (New; did not exist.)
              uint8_t  padding01[3];   // PS2: (New; did not exist.)
              uint32_t unknown00;      // PS2: (New; did not exist.)
              uint8_t  unknown01;      // PS2: (New; did not exist.)
              uint8_t  padding02[3];   // PS2: (New; did not exist.)
);
