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

#include "0x0b6_chat_name.h"

#include "common/async.h"
#include "common/database.h"
#include "common/ipc_structs.h"
#include "common/settings.h"
#include "entities/charentity.h"
#include "ipc_client.h"
#include "lua/luautils.h"
#include "packets/message_basic.h"
#include "utils/jailutils.h"

auto GP_CLI_COMMAND_CHAT_NAME::validate(MapSession* PSession, const CCharEntity* PChar) const -> PacketValidationResult
{
    return PacketValidator()
        .mustEqual(unknown00, 3, "unknown00 not 3")
        .mustEqual(unknown01, 0, "unknown01 not 0");
}

void GP_CLI_COMMAND_CHAT_NAME::process(MapSession* PSession, CCharEntity* PChar) const
{
    if (jailutils::InPrison(PChar))
    {
        PChar->pushPacket<CMessageBasicPacket>(PChar, PChar, 0, 0, MSGBASIC_CANNOT_USE_IN_AREA);
        return;
    }

    const auto recipientName = db::escapeString(asStringFromUntrustedSource(sName, sizeof(sName)));
    const auto rawMessage    = asStringFromUntrustedSource(Mes, sizeof(Mes));

    if (strcmp(recipientName.c_str(), "_CUSTOM_MENU") == 0 &&
        luautils::HasCustomMenuContext(PChar))
    {
        luautils::HandleCustomMenu(PChar, rawMessage);
        return;
    }

    message::send(ipc::ChatMessageTell{
        .senderId      = PChar->id,
        .senderName    = PChar->getName(),
        .recipientName = recipientName,
        .message       = rawMessage,
        .zoneId        = PChar->getZone(),
        .gmLevel       = PChar->m_GMlevel,
    });

    if (settings::get<bool>("map.AUDIT_CHAT") && settings::get<bool>("map.AUDIT_TELL"))
    {
        // clang-format off
        Async::getInstance()->submit([name = PChar->getName(), zoneId = PChar->getZone(), recipient = recipientName, rawMessage]()
        {
            const auto query = "INSERT INTO audit_chat (speaker, type, zoneid, recipient, message, datetime) VALUES(?, 'TELL', ?, ?, ?, current_timestamp())";
            if (!db::preparedStmt(query, name, zoneId, recipient, rawMessage))
            {
                ShowError("Failed to insert TELL audit_chat record for player '%s'", name);
            }
        });
        // clang-format on
    }
}
