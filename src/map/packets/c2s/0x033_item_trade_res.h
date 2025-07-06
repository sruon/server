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

enum class GP_CLI_COMMAND_ITEM_TRADE_RES_KIND : uint32_t
{
    Start      = 0,
    Cancel     = 1,
    Make       = 2,
    MakeCancel = 3,
    // There's more values but the client only uses 0-3 for this packet
};

// https://github.com/atom0s/XiPackets/tree/main/world/client/0x0033
// This packet is sent by the client when responding to a trade request.
GP_CLI_PACKET(GP_CLI_COMMAND_ITEM_TRADE_RES,
              uint32_t Kind;         // PS2: Kind
              uint16_t TradeCounter; // PS2: TradeCounter
);
