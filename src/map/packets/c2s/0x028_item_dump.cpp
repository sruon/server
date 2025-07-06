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

#include "0x028_item_dump.h"

#include "entities/charentity.h"
#include "items.h"
#include "items/item_linkshell.h"
#include "linkshell.h"
#include "packets/message_standard.h"
#include "utils/charutils.h"

auto GP_CLI_COMMAND_ITEM_DUMP::validate(MapSession* PSession, const CCharEntity* PChar) const -> PacketValidationResult
{
    return PacketValidator()
        .range("ItemNum", ItemNum, 0, 99);
}

void GP_CLI_COMMAND_ITEM_DUMP::process(MapSession* PSession, CCharEntity* PChar) const
{
    CItem* PItem = PChar->getStorage(Category)->GetItem(ItemIndex);
    if (PItem == nullptr)
    {
        return;
    }

    const uint16 itemId = PItem->getID();

    if (Category >= CONTAINER_ID::MAX_CONTAINER_ID)
    {
        ShowWarning("GP_CLI_COMMAND_ITEM_DUMP: Invalid container ID passed to packet %u by %s", Category, PChar->getName());
        return;
    }

    if (PItem->isSubType(ITEM_LOCKED))
    {
        ShowWarning("GP_CLI_COMMAND_ITEM_DUMP: Attempt of removal of LOCKED item from slot %u", ItemIndex);
        return;
    }

    if (PItem->isStorageSlip())
    {
        int slipData = 0;
        for (int i = 0; i < CItem::extra_size; i++)
        {
            slipData += PItem->m_extra[i];
        }

        if (slipData != 0)
        {
            PChar->pushPacket<CMessageStandardPacket>(MsgStd::CannotBeProcessed);
            return;
        }
    }

    // Break linkshell if the main shell was disposed of.
    if (auto* ItemLinkshell = dynamic_cast<CItemLinkshell*>(PItem))
    {
        if (ItemLinkshell->GetLSType() == LSTYPE_LINKSHELL)
        {
            const uint32 lsid       = ItemLinkshell->GetLSID();
            CLinkshell*  PLinkshell = linkshell::GetLinkshell(lsid);
            if (!PLinkshell)
            {
                PLinkshell = linkshell::LoadLinkshell(lsid);
            }
            PLinkshell->BreakLinkshell();
            linkshell::UnloadLinkshell(lsid);
        }
    }

    // Linkshells (other than Linkpearls and Pearlsacks) and temporary items cannot be stored in the Recycle Bin.
    if (!settings::get<bool>("map.ENABLE_ITEM_RECYCLE_BIN") || itemId == ITEMID::LINKSHELL || Category == CONTAINER_ID::LOC_TEMPITEMS)
    {
        charutils::DropItem(PChar, Category, ItemIndex, ItemNum, itemId);
        return;
    }

    // Otherwise, to the recycle bin!
    charutils::AddItemToRecycleBin(PChar, Category, Category, ItemNum);
}
