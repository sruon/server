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

#include "0x064_scenarioitem.h"

#include "entities/charentity.h"
#include "utils/charutils.h"

auto GP_CLI_COMMAND_SCENARIOITEM::validate(MapSession* PSession, const CCharEntity* PChar) const -> PacketValidationResult
{
    return PacketValidator()
        .mustEqual(UniqueNo, PChar->id, "Character ID mismatch")
        .mustEqual(ActIndex, PChar->targid, "Targid mismatch")
        .range("TableIndex", TableIndex, 0, PChar->keys.tables.size() - 1);
}

void GP_CLI_COMMAND_SCENARIOITEM::process(MapSession* PSession, CCharEntity* PChar) const
{
    for (int i = 0; i < 0x40; i++)
    {
        uint32_t flagIndex  = i / 4; // Which uint32_t in the array (0-15)
        uint32_t byteInFlag = i % 4; // Which byte within that uint32_t (0-3)

        if (flagIndex < 16)
        {
            uint8_t flagByte = (LookItemFlag[flagIndex] >> (byteInFlag * 8)) & 0xFF;

            // Set each bit in the seenList based on the corresponding bit in flagByte
            PChar->keys.tables[TableIndex].seenList.set(i * 8 + 0, flagByte & 0x01);
            PChar->keys.tables[TableIndex].seenList.set(i * 8 + 1, flagByte & 0x02);
            PChar->keys.tables[TableIndex].seenList.set(i * 8 + 2, flagByte & 0x04);
            PChar->keys.tables[TableIndex].seenList.set(i * 8 + 3, flagByte & 0x08);
            PChar->keys.tables[TableIndex].seenList.set(i * 8 + 4, flagByte & 0x10);
            PChar->keys.tables[TableIndex].seenList.set(i * 8 + 5, flagByte & 0x20);
            PChar->keys.tables[TableIndex].seenList.set(i * 8 + 6, flagByte & 0x40);
            PChar->keys.tables[TableIndex].seenList.set(i * 8 + 7, flagByte & 0x80);
        }
    }

    charutils::SaveKeyItems(PChar);
}
