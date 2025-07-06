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

#include "0x04e_auc.h"

#include "utils/auctionutils.h"

auto GP_CLI_COMMAND_AUC::validate(MapSession* PSession, const CCharEntity* PChar) const -> PacketValidationResult
{
    // TODO: Handler need refactor before we can validate at this level.
    return PacketValidator()
        .oneOf<GP_CLI_COMMAND_AUC_COMMAND>(Command);
}

void GP_CLI_COMMAND_AUC::process(MapSession* PSession, CCharEntity* PChar) const
{
    auctionutils::HandlePacket(PChar, *this);
}
