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

#include "0x051_equipset_set.h"

#include "entities/charentity.h"
#include "lua/luautils.h"
#include "utils/charutils.h"

namespace
{
    const std::set validContainers = {
        LOC_INVENTORY, LOC_WARDROBE, LOC_WARDROBE2, LOC_WARDROBE3, LOC_WARDROBE4,
        LOC_WARDROBE5, LOC_WARDROBE6, LOC_WARDROBE7, LOC_WARDROBE8
    };
}
auto GP_CLI_COMMAND_EQUIPSET_SET::validate(MapSession* PSession, const CCharEntity* PChar) const -> PacketValidationResult
{
    return PacketValidator()
        .isNormalStatus(PChar)
        .range("Count", Count, 1, 16);
}

void GP_CLI_COMMAND_EQUIPSET_SET::process(MapSession* PSession, CCharEntity* PChar) const
{
    for (uint8 i = 0; i < Count; i++)
    {
        auto equipment = Equipment[i];
        if (validContainers.contains(static_cast<CONTAINER_ID>(equipment.Category)))
        {
            charutils::EquipItem(PChar, Equipment[i].ItemIndex, Equipment[i].EquipKind, equipment.Category);
        }
    }

    PChar->RequestPersist(CHAR_PERSIST::EQUIP);
    luautils::CheckForGearSet(PChar); // check for gear set on gear change
    PChar->UpdateHealth();
    PChar->retriggerLatents = true; // retrigger all latents later because our gear has changed
}
