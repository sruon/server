//
// Created by sruon on 5/18/2025.
//

#pragma once

#include "common/cbasetypes.h"
#include "map_party.h"

enum class PartyMemberType : uint8;
class CBattleEntity;
class CCharEntity;

class CCharParty::IpcHelper
{
public:
    static std::unique_ptr<IpcHelper> Create(const CCharParty& party)
    {
        return std::unique_ptr<IpcHelper>(new IpcHelper(party));
    }

    void SetLeader(const CCharEntity* PChar) const;
    void SetLeader(const uint32 UniqueNo) const;

    void SetSyncTarget(const CCharEntity* PChar) const;
    void SetSyncTarget(const uint32 UniqueNo) const;

    void SetQuartermaster(const CCharEntity* PChar) const;
    void SetQuartermaster(const uint32 UniqueNo) const;

    void AddMember(const uint32 UniqueNo, const PartyMemberType Type, const uint16 ZoneId) const;
    void AddMember(CBattleEntity* PEntity) const;

    void RemoveMember(const uint32 UniqueNo) const;
    void RemoveMember(CBattleEntity* PEntity) const;

    void NotifyKick(const uint32 UniqueNo) const;
    void NotifyKick(CCharEntity* PEntity) const;

protected:
    IpcHelper(const CCharParty& party)
    : m_Party(party)
    {
    }

private:
    const CCharParty& m_Party;
    friend class CCharParty;
};