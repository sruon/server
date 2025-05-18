//
// Created by sruon on 5/17/2025.
//

#pragma once

#include "common/ipc_structs.h"
#include "entities/charentity.h"
#include "entities/trustentity.h"
#include "ipc_client.h"
#include "packets/party_define.h"
#include "packets/party_effects.h"
#include "packets/party_member_update.h"

class CCharParty
{
public:
    // All map->world communications go through this
    class IpcHelper;

    CCharParty(const ipc::PartyUpdate& message);
    const IpcHelper& ipc() const;

    bool IsFull() const;

    bool HasOnlyOneMember() const;

    timer::time_point GetTimeLastMemberJoined() const;

    bool HasTrusts();

    void                        broadcast();
    void                        update(const ipc::PartyUpdate& message);
    std::vector<CBattleEntity*> GetMembersWithTrusts() const;
    std::vector<CCharEntity*>   GetMembers() const;
    std::vector<CCharEntity*>   GetMembers(uint16 zoneId) const;
    CCharEntity*                GetLeader() const;
    CCharEntity*                GetQuartermaster() const;
    uint32                      GetPartyID() const
    {
        return m_PartyId;
    }

    CCharEntity* GetSyncTarget() const;
    uint16       GetFlagsForMember(CCharEntity* PChar);
    void         PushEffectsPacket();
    void         PushPacket(uint32 senderID, uint16 ZoneID, const std::unique_ptr<CBasicPacket>& packet);
    void         EffectsChanged()
    {
        m_EffectsChanged = true;
    }
    CCharEntity* GetMemberByName(const std::string& memberName); // Returns entity pointer for member name string
    void         ForEveryMember(std::function<void(CCharEntity*)> func);
    void         ForEveryMemberWithTrusts(std::function<void(CBattleEntity*)> func);
    void         ForEveryAllianceMember(std::function<void(CCharEntity*)> func);

private:
    void                       addMember(PartyMemberData& data);
    void                       delMember(const PartyMember& member);
    std::vector<PartyMember>   members_;
    uint32                     m_PartyId               = 0;
    uint32                     m_LeaderUniqueNo        = 0;
    uint32                     m_QuartermasterUniqueNo = 0;
    uint32                     m_SyncTargetUniqueNo    = 0;
    timer::time_point          m_LastJoined            = timer::now();
    bool                       m_EffectsChanged        = false;
    std::unique_ptr<IpcHelper> m_pIpcHelper;
};

#include "map_party_ipc_helper.h"