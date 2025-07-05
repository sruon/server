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

#include "0x070_group_breakup.h"

#include "alliance.h"
#include "entities/charentity.h"
#include "party.h"

auto GP_CLI_COMMAND_GROUP_BREAKUP::validate(MapSession* PSession, const CCharEntity* PChar) const -> PacketValidationResult
{
    return PacketValidator()
        .oneOf<GP_CLI_COMMAND_GROUP_BREAKUP_KIND>(Kind)
        .mustNotEqual(PChar->PParty, nullptr, "Character does not have a party");
}

void GP_CLI_COMMAND_GROUP_BREAKUP::process(MapSession* PSession, CCharEntity* PChar) const
{
    if (PChar->PParty->GetLeader() != PChar)
    {
        return;
    }

    switch (static_cast<GP_CLI_COMMAND_GROUP_BREAKUP_KIND>(Kind))
    {
        case GP_CLI_COMMAND_GROUP_BREAKUP_KIND::Party:
        {
            // Party leader may disband party if not an alliance member
            if (PChar->PParty->m_PAlliance == nullptr)
            {
                ShowDebug("%s is disbanding the party (pcmd breakup)", PChar->getName());
                PChar->PParty->DisbandParty();
                ShowDebug("%s party has been disbanded (pcmd breakup)", PChar->getName());
            }
        }
        break;
        case GP_CLI_COMMAND_GROUP_BREAKUP_KIND::Alliance:
        {
            // Only alliance leader may dissolve the entire alliance
            if (PChar->PParty->m_PAlliance && PChar->PParty->m_PAlliance->getMainParty() == PChar->PParty)
            {
                ShowDebug("%s is disbanding the alliance (acmd breakup)", PChar->getName());
                PChar->PParty->m_PAlliance->dissolveAlliance();
                ShowDebug("%s alliance has been disbanded (acmd breakup)", PChar->getName());
            }
        }
        break;
    }
}
