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

#include "0x029_item_move.h"

#include "entities/charentity.h"
#include "packets/inventory_finish.h"
#include "packets/inventory_item.h"
#include "utils/charutils.h"

auto GP_CLI_COMMAND_ITEM_MOVE::validate(MapSession* PSession, const CCharEntity* PChar) const -> PacketValidationResult
{
    return PacketValidator()
        .range("Category1", Category1, 0, CONTAINER_ID::MAX_CONTAINER_ID - 1)
        .range("Category2", Category2, 0, CONTAINER_ID::MAX_CONTAINER_ID - 1)
        .range("ItemNum", ItemNum, 1, 99);
}

void GP_CLI_COMMAND_ITEM_MOVE::process(MapSession* PSession, CCharEntity* PChar) const
{
    CItem* PItem = PChar->getStorage(Category1)->GetItem(ItemIndex1);

    if (PItem == nullptr || PItem->isSubType(ITEM_LOCKED))
    {
        if (PItem == nullptr)
        {
            ShowWarning("GP_CLI_COMMAND_ITEM_MOVE: Trying to move nullptr item from location %u slot %u to location %u slot %u of quan %u ",
                        Category1, ItemIndex1, Category2, ItemIndex2, ItemNum);
        }
        else
        {
            ShowWarning("GP_CLI_COMMAND_ITEM_MOVE: Trying to move LOCKED item %i from location %u slot %u to location %u slot %u of quan %u ",
                        PItem->getID(), Category1, ItemIndex1, Category2, ItemIndex2, ItemNum);
        }

        const uint8 size = PChar->getStorage(Category1)->GetSize();
        for (uint8 slotID = 0; slotID <= size; ++slotID)
        {
            CItem* PSlotItem = PChar->getStorage(Category1)->GetItem(slotID);
            if (PSlotItem != nullptr)
            {
                PChar->pushPacket<CInventoryItemPacket>(PSlotItem, Category1, slotID);
            }
        }
        PChar->pushPacket<CInventoryFinishPacket>();

        return;
    }

    if (PItem->getQuantity() - PItem->getReserve() < ItemNum)
    {
        ShowWarning("GP_CLI_COMMAND_ITEM_MOVE: Trying to move too much quantity from location %u slot %u", Category1, ItemIndex1);
        return;
    }

    uint32 NewQuantity = PItem->getQuantity() - ItemNum;

    if (NewQuantity != 0) // split item stack
    {
        if (charutils::AddItem(PChar, Category2, PItem->getID(), ItemNum) != ERROR_SLOTID)
        {
            charutils::UpdateItem(PChar, Category1, ItemIndex1, -static_cast<int32>(ItemNum));
        }
    }
    else // move stack / combine items into stack
    {
        if (ItemIndex2 < 82) // 80 + 1
        {
            ShowDebug("GP_CLI_COMMAND_ITEM_MOVE: Trying to unite items", Category1, ItemIndex1);
            CItem* PItem2 = PChar->getStorage(Category2)->GetItem(ItemIndex2);

            if ((PItem2 != nullptr) && (PItem2->getID() == PItem->getID()) && (PItem2->getQuantity() < PItem2->getStackSize()) &&
                !PItem2->isSubType(ITEM_LOCKED) && (PItem2->getReserve() == 0))
            {
                const uint32 totalQty = PItem->getQuantity() + PItem2->getQuantity();
                uint32       moveQty  = 0;

                if (totalQty >= PItem2->getStackSize())
                {
                    moveQty = PItem2->getStackSize() - PItem2->getQuantity();
                }
                else
                {
                    moveQty = PItem->getQuantity();
                }
                if (moveQty > 0)
                {
                    charutils::UpdateItem(PChar, Category2, ItemIndex2, moveQty);
                    charutils::UpdateItem(PChar, Category1, ItemIndex1, -static_cast<int32>(moveQty));
                }
            }

            return;
        }

        if (uint8 newSlotId = PChar->getStorage(Category2)->InsertItem(PItem); newSlotId != ERROR_SLOTID)
        {
            const auto rset = db::preparedStmt("UPDATE char_inventory SET location = ?, slot = ? WHERE charid = ? AND location = ? AND slot = ?",
                                               Category2, newSlotId, PChar->id, Category1, ItemIndex1);
            if (rset && rset->rowsAffected())
            {
                PChar->getStorage(Category1)->InsertItem(nullptr, ItemIndex1);

                PChar->pushPacket<CInventoryItemPacket>(nullptr, Category1, ItemIndex1);
                PChar->pushPacket<CInventoryItemPacket>(PItem, Category2, newSlotId);
            }
            else
            {
                PChar->getStorage(Category2)->InsertItem(nullptr, newSlotId);
                PChar->getStorage(Category1)->InsertItem(PItem, ItemIndex1);
            }
        }
        else
        {
            // Client assumed the location was not full when it is
            // Resend the packets to inform the client of the storage sizes
            const uint8 size = PChar->getStorage(Category2)->GetSize();
            for (uint8 slotID = 0; slotID <= size; ++slotID)
            {
                CItem* PSlotItem = PChar->getStorage(Category2)->GetItem(slotID);
                if (PSlotItem != nullptr)
                {
                    PChar->pushPacket<CInventoryItemPacket>(PSlotItem, Category2, slotID);
                }
            }
            PChar->pushPacket<CInventoryFinishPacket>();

            ShowError("SmallPacket0x29: Location %u Slot %u is full", Category2, ItemIndex2);
            return;
        }
    }

    PChar->pushPacket<CInventoryFinishPacket>();
}
