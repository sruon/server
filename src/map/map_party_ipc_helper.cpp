//
// Created by sruon on 5/18/2025.
//

#include "map_party_ipc_helper.h"

#include "entities/charentity.h"
#include "entities/trustentity.h"
#include "ipc_client.h"

// Tell the world server the party leader is changing
void CCharParty::IpcHelper::SetLeader(const CCharEntity* PChar) const
{
    SetLeader(PChar ? PChar->id : 0);
}

// Tell the world server the party leader is changing
void CCharParty::IpcHelper::SetLeader(const uint32 UniqueNo) const
{
    message::send(ipc::PartySetLeader{
        .partyId = m_Party.GetPartyId(),
        .charId  = UniqueNo,
    });
}

// Tell the world server the party leader is changing
void CCharParty::IpcHelper::SetLeader(const std::string charName) const
{
    message::send(ipc::PartySetLeader{
        .partyId  = m_Party.GetPartyId(),
        .charName = charName,
    });
}

// Tell the world server the sync target is changing
void CCharParty::IpcHelper::SetSyncTarget(const CCharEntity* PChar) const
{
    SetSyncTarget(PChar ? PChar->id : 0);
}

void CCharParty::IpcHelper::ClearSyncTarget(const MsgStd Reason) const
{
    message::send(ipc::PartySetSyncTarget{
        .partyId = m_Party.GetPartyId(),
        .charId  = 0,
        .reason  = Reason,
    });
}

void CCharParty::IpcHelper::SetSyncTarget(const std::string& CharName) const
{
    message::send(ipc::PartySetSyncTarget{
        .partyId  = m_Party.GetPartyId(),
        .charName = CharName,
    });
}

// Tell the world server the sync target is changing
void CCharParty::IpcHelper::SetSyncTarget(const uint32 UniqueNo) const
{
    message::send(ipc::PartySetSyncTarget{
        .partyId = m_Party.GetPartyId(),
        .charId  = UniqueNo,
    });
}

// Tell the world server the quartermaster is changing
void CCharParty::IpcHelper::SetQuartermaster(const CCharEntity* PChar) const
{
    SetQuartermaster(PChar ? PChar->id : 0);
}

// Tell the world server the quartermaster is changing
void CCharParty::IpcHelper::SetQuartermaster(const uint32 UniqueNo) const
{
    message::send(ipc::PartySetQuartermaster{
        .partyId = m_Party.GetPartyId(),
        .charId  = UniqueNo,
    });
}

void CCharParty::IpcHelper::SetQuartermaster(const std::string charName) const
{
    message::send(ipc::PartySetQuartermaster{
        .partyId  = m_Party.GetPartyId(),
        .charName = charName,
    });
}

// Tell the world server we'd like to add a member
void CCharParty::IpcHelper::AddMember(const uint32 UniqueNo, const PartyMemberType Type, const uint16 ZoneId) const
{
    message::send(ipc::PartyAddMember{
        .partyId = m_Party.GetPartyId(),
        .charId  = UniqueNo,
        .type    = Type,
        .zoneId  = ZoneId,
    });
}

// Tell the world server we'd like to add a member
void CCharParty::IpcHelper::AddMember(CBattleEntity* PEntity) const
{
    if (const auto* PChar = dynamic_cast<CCharEntity*>(PEntity))
    {
        AddMember(PChar->id, PartyMemberType::Player, PChar->getZone());
    }
    else if (const auto* PTrust = dynamic_cast<CTrustEntity*>(PEntity))
    {
        AddMember(PTrust->id, PartyMemberType::Trust, PTrust->getZone());
    }
}

// Tell the world server we'd like to remove a member
void CCharParty::IpcHelper::RemoveMember(const uint32 UniqueNo) const
{
    message::send(ipc::PartyRemoveMember{
        .partyId = m_Party.GetPartyId(),
        .charId  = UniqueNo,
    });
}

// Tell the world server we'd like to remove a member
void CCharParty::IpcHelper::RemoveMember(CBattleEntity* PEntity) const
{
    if (PEntity)
    {
        RemoveMember(PEntity->id);
    }
}

// Tell the world server we'd like to notify a member they've been removed
void CCharParty::IpcHelper::NotifyKick(const uint32 UniqueNo) const
{
    message::send(ipc::PlayerKick{
        .victimId = UniqueNo });
}

// Tell the world server we'd like to notify a member they've been removed
void CCharParty::IpcHelper::NotifyKick(CCharEntity* PEntity) const
{
    if (PEntity)
    {
        NotifyKick(PEntity->id);
    }
}

// Tell the world server we'd like to disband the party
void CCharParty::IpcHelper::Disband() const
{
    message::send(ipc::PartyDisband{
        .partyId = m_Party.GetPartyId(),
    });
}