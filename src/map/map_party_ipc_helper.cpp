//
// Created by sruon on 5/18/2025.
//

#include "map_party_ipc_helper.h"

void CCharParty::IpcHelper::SetLeader(const CCharEntity* PChar) const
{
    SetLeader(PChar ? PChar->id : 0);
}

void CCharParty::IpcHelper::SetLeader(const uint32 UniqueNo) const
{
    message::send(ipc::PartySetLeader{
        .partyId = m_Party.GetPartyID(),
        .charId  = UniqueNo,
    });
}

void CCharParty::IpcHelper::SetSyncTarget(const CCharEntity* PChar) const
{
    SetSyncTarget(PChar ? PChar->id : 0);
}

void CCharParty::IpcHelper::SetSyncTarget(const uint32 UniqueNo) const
{
    message::send(ipc::PartySetSyncTarget{
        .partyId = m_Party.GetPartyID(),
        .charId  = UniqueNo,
    });
}

void CCharParty::IpcHelper::SetQuartermaster(const CCharEntity* PChar) const
{
    SetQuartermaster(PChar ? PChar->id : 0);
}

void CCharParty::IpcHelper::SetQuartermaster(const uint32 UniqueNo) const
{
    message::send(ipc::PartySetQuartermaster{
        .partyId = m_Party.GetPartyID(),
        .charId  = UniqueNo,
    });
}

void CCharParty::IpcHelper::AddMember(const uint32 UniqueNo, const PartyMemberType Type, const uint16 ZoneId) const
{
    message::send(ipc::PartyAddMember{
        .partyId = m_Party.GetPartyID(),
        .charId  = UniqueNo,
        .type    = Type,
        .zoneId  = ZoneId,
    });
}

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

void CCharParty::IpcHelper::RemoveMember(const uint32 UniqueNo) const
{
    message::send(ipc::PartyRemoveMember{
        .partyId = m_Party.GetPartyID(),
        .charId  = UniqueNo,
    });
}

void CCharParty::IpcHelper::RemoveMember(CBattleEntity* PEntity) const
{
    if (PEntity)
    {
        RemoveMember(PEntity->id);
    }
}

void CCharParty::IpcHelper::NotifyKick(const uint32 UniqueNo) const
{
    message::send(ipc::PlayerKick{
        .victimId = UniqueNo
    });
}

void CCharParty::IpcHelper::NotifyKick(CCharEntity* PEntity) const
{
    if (PEntity)
    {
        NotifyKick(PEntity->id);
    }
}