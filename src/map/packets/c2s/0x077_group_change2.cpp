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

#include "0x077_group_change2.h"

#include "common/database.h"
#include "entities/charentity.h"
#include "ipc_client.h"
#include "items/item_linkshell.h"
#include "linkshell.h"

auto GP_CLI_COMMAND_GROUP_CHANGE2::validate(MapSession* PSession, const CCharEntity* PChar) const -> PacketValidationResult
{
    return PacketValidator()
        .oneOf<GP_CLI_COMMAND_GROUP_CHANGE2_KIND>(Kind)
        .oneOf<GP_CLI_COMMAND_GROUP_CHANGE2_CHANGEKIND>(ChangeKind);
}

void GP_CLI_COMMAND_GROUP_CHANGE2::process(MapSession* PSession, CCharEntity* PChar) const
{
    const auto memberName = db::escapeString(asStringFromUntrustedSource(sName, sizeof(sName)));

    switch (static_cast<GP_CLI_COMMAND_GROUP_CHANGE2_KIND>(Kind))
    {
        case GP_CLI_COMMAND_GROUP_CHANGE2_KIND::Party:
        {
            if (!PChar->PParty)
            {
                return;
            }

            if (PChar->PParty->GetLeader() == PChar)
            {
                ShowDebug(fmt::format("(Party) Altering permissions of {} to {}", memberName, ChangeKind));
                PChar->PParty->AssignPartyRole(memberName, ChangeKind);
            }
        }
        break;
        case GP_CLI_COMMAND_GROUP_CHANGE2_KIND::Linkshell1:
        {
            if (!PChar->PLinkshell1)
            {
                return;
            }

            if (auto* PItemLinkshell = reinterpret_cast<CItemLinkshell*>(PChar->getEquip(SLOT_LINK1)))
            {
                message::send(ipc::LinkshellRankChange{
                    .requesterId   = PChar->id,
                    .requesterRank = PItemLinkshell->GetLSType(),
                    .memberName    = memberName,
                    .linkshellId   = PChar->PLinkshell1->getID(),
                    .newRank       = ChangeKind,
                });
            }
        }
        break;
        case GP_CLI_COMMAND_GROUP_CHANGE2_KIND::Linkshell2:
        {
            if (!PChar->PLinkshell2)
            {
                return;
            }

            if (auto* PItemLinkshell = reinterpret_cast<CItemLinkshell*>(PChar->getEquip(SLOT_LINK2)))
            {
                message::send(ipc::LinkshellRankChange{
                    .requesterId   = PChar->id,
                    .requesterRank = PItemLinkshell->GetLSType(),
                    .memberName    = memberName,
                    .linkshellId   = PChar->PLinkshell2->getID(),
                    .newRank       = ChangeKind,
                });
            }
        }
        break;
        case GP_CLI_COMMAND_GROUP_CHANGE2_KIND::Alliance:
        {
            if (!PChar->PParty || !PChar->PParty->m_PAlliance)
            {
                return;
            }

            if (PChar->PParty->GetLeader() == PChar && PChar->PParty->m_PAlliance->getMainParty() == PChar->PParty)
            {
                ShowDebug(fmt::format("(Alliance) Changing leader to {}", memberName));
                PChar->PParty->m_PAlliance->assignAllianceLeader(memberName);

                message::send(ipc::AllianceReload{
                    .allianceId = PChar->PParty->m_PAlliance->m_AllianceID,
                });
            }
        }
        break;
    }
}
