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

#include "party/ipc_helper.h"
#include "entities/charentity.h"
#include "entities/trustentity.h"
#include "ipc_client.h"

// Tell the world server the party leader is changing
void CCharParty::IpcHelper::setLeader(const uint32 UniqueNo) const
{
    message::send(ipc::PartyEvent{
        .partyId = m_Party.getPartyId(),
        .payload = LeaderSetMessage{
            .charId = UniqueNo,
        },
    });
}

// Tell the world server the party leader is changing
void CCharParty::IpcHelper::setLeader(const std::string& charName) const
{
    message::send(ipc::PartyEvent{
        .partyId = m_Party.getPartyId(),
        .payload = LeaderSetMessage{
            .charName = charName,
        },
    });
}

void CCharParty::IpcHelper::clearSyncTarget(const MsgStd Reason) const
{
    message::send(ipc::PartyEvent{
        .partyId = m_Party.getPartyId(),
        .payload = SyncTargetSetMessage{
            .charId = 0,
            .reason = Reason,
        },
    });
}

void CCharParty::IpcHelper::setSyncTarget(const std::string& charName) const
{
    message::send(ipc::PartyEvent{
        .partyId = m_Party.getPartyId(),
        .payload = SyncTargetSetMessage{
            .charName = charName,
        },
    });
}

// Tell the world server the sync target is changing
void CCharParty::IpcHelper::setSyncTarget(const uint32 UniqueNo) const
{
    message::send(ipc::PartyEvent{
        .partyId = m_Party.getPartyId(),
        .payload = SyncTargetSetMessage{
            .charId = UniqueNo,
        },
    });
}

// Tell the world server the quartermaster is changing
void CCharParty::IpcHelper::setQuartermaster(const uint32 UniqueNo) const
{
    message::send(ipc::PartyEvent{
        .partyId = m_Party.getPartyId(),
        .payload = QuartermasterSetMessage{
            .charId = UniqueNo,
        },
    });
}

void CCharParty::IpcHelper::setQuartermaster(const std::string& charName) const
{
    message::send(ipc::PartyEvent{
        .partyId = m_Party.getPartyId(),
        .payload = QuartermasterSetMessage{
            .charName = charName,
        },
    });
}

// Tell the world server we'd like to add a member
void CCharParty::IpcHelper::addMember(const uint32 UniqueNo, const PartyMemberType Type) const
{
    message::send(ipc::PartyEvent{
        .partyId = m_Party.getPartyId(),
        .payload = MemberAddMessage{
            .charId = UniqueNo,
            .type   = Type,
        },
    });
}

// Tell the world server we'd like to remove a member
void CCharParty::IpcHelper::removeMember(const uint32 UniqueNo) const
{
    message::send(ipc::PartyEvent{
        .partyId = m_Party.getPartyId(),
        .payload = MemberRemoveMessage{
            .charId = UniqueNo,
        },
    });
}

// Tell the world server we'd like to remove a member
void CCharParty::IpcHelper::removeMember(const std::string& charName) const
{
    message::send(ipc::PartyEvent{
        .partyId = m_Party.getPartyId(),
        .payload = MemberRemoveMessage{
            .charName = charName,
        },
    });
}

// Tell the world server we'd like to disband the party
void CCharParty::IpcHelper::disband() const
{
    message::send(ipc::PartyEvent{
        .partyId = m_Party.getPartyId(),
        .payload = DisbandMessage{},
    });
}
