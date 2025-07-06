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

#include "0x096_combine_ask.h"

#include "common/logging.h"
#include "common/timer.h"
#include "entities/charentity.h"
#include "packets/message_basic.h"
#include "packets/message_standard.h"
#include "packets/trade_action.h"
#include "trade_container.h"
#include "universal_container.h"
#include "utils/charutils.h"
#include "utils/jailutils.h"
#include "utils/synthutils.h"

auto GP_CLI_COMMAND_COMBINE_ASK::validate(MapSession* PSession, const CCharEntity* PChar) const -> PacketValidationResult
{
    return PacketValidator()
        .isNotCrafting(PChar)
        .range("Items", Items, 1, 8);
}

void GP_CLI_COMMAND_COMBINE_ASK::process(MapSession* PSession, CCharEntity* PChar) const
{
    if (jailutils::InPrison(PChar))
    {
        // Prevent crafting in prison
        PChar->pushPacket<CMessageBasicPacket>(PChar, PChar, 0, 0, MSGBASIC_CANNOT_USE_IN_AREA);
        return;
    }

    // Force full synth duration wait no matter the synth animation length
    // Thus players can synth on whatever fps they want
    if (PChar->m_LastSynthTime + 15s > timer::now())
    {
        PChar->pushPacket<CMessageBasicPacket>(PChar, PChar, 0, 0, 94);
        return;
    }

    // NOTE: This section is intended to be temporary to ensure that duping shenanigans aren't possible.
    // It should be replaced by something more robust or more stateful as soon as is reasonable
    CCharEntity* PTarget = static_cast<CCharEntity*>(PChar->GetEntity(PChar->TradePending.targid, TYPE_PC));

    // Clear pending trades on synthesis start
    if (PTarget && PChar->TradePending.id == PTarget->id)
    {
        PChar->TradePending.clean();
        PTarget->TradePending.clean();
    }

    // Clears out trade session and blocks synthesis at any point in trade process after accepting
    // trade request.
    if (PChar->UContainer->GetType() != UCONTAINER_EMPTY)
    {
        if (PTarget)
        {
            ShowDebug("%s trade request with %s was canceled because %s tried to craft.",
                      PChar->getName(), PTarget->getName(), PChar->getName());

            PTarget->TradePending.clean();
            PTarget->UContainer->Clean();
            PTarget->pushPacket<CTradeActionPacket>(PChar, 0x01);
            PChar->pushPacket<CTradeActionPacket>(PTarget, 0x01);
        }
        PChar->pushPacket<CMessageStandardPacket>(MsgStd::CannotBeProcessed);
        PChar->TradePending.clean();
        PChar->UContainer->Clean();
        return;
    }
    // End temporary additions

    PChar->CraftContainer->Clean();

    // Verify and set crystal
    auto PItem = PChar->getStorage(LOC_INVENTORY)->GetItem(CrystalIdx);
    if (!PItem || Crystal != PItem->getID() || PItem->getQuantity() == 0)
    {
        PChar->pushPacket<CMessageBasicPacket>(PChar, PChar, 0, 0, MSGBASIC_CANNOT_USE_IN_AREA);
        return;
    }

    PChar->CraftContainer->setItem(0, Crystal, CrystalIdx, 0);

    // Set ingredients
    std::vector<uint8> slotQty(MAX_CONTAINER_SIZE);
    for (int32 SlotID = 0; SlotID < Items; ++SlotID)
    {
        slotQty[TableNo[SlotID]]++;

        auto* PSlotItem = PChar->getStorage(LOC_INVENTORY)->GetItem(TableNo[SlotID]);
        if (PSlotItem && PSlotItem->getID() == ItemNo[SlotID] && slotQty[TableNo[SlotID]] <= (PSlotItem->getQuantity() - PSlotItem->getReserve()))
        {
            PChar->CraftContainer->setItem(SlotID + 1, ItemNo[SlotID], TableNo[SlotID], 1);
        }
    }

    synthutils::startSynth(PChar);
}
