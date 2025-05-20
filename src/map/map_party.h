//
// Created by sruon on 5/17/2025.
//

#pragma once

#include "common/ipc.h"
#include "common/cbasetypes.h"

class CCharEntity;
class CBattleEntity;
enum class PartyFlag : uint16;

// This is a read-only view of a party of CCharEntity
// Updates are only permitted through the IPC interface
// The nested IpcHelper class is used to send messages to the world server
class CCharParty
{
public:
    // All map->world communications go through the IpcHelper
    class IpcHelper;
    auto ipc() const -> const IpcHelper&;

    static std::unique_ptr<CCharParty> Create(const ipc::PartyUpdate& message)
    {
        return std::unique_ptr<CCharParty>(new CCharParty(message));
    }

    ~CCharParty();

    CCharParty(const CCharParty&)            = delete;
    CCharParty& operator=(const CCharParty&) = delete;
    CCharParty(CCharParty&&)                 = delete;
    CCharParty& operator=(CCharParty&&)      = delete;

    void applySync(CCharEntity* PChar) const;

    auto GetPartyId() const -> uint32;

    // Helpers
    bool IsFull() const;
    bool HasOnlyOneMember() const;
    auto GetTimeLastMemberJoined() const -> timer::time_point;
    bool HasTrusts();
    bool IsTrustOnlyParty() const;
    auto GetFlagsForMember(const CCharEntity* PChar) const -> uint16;
    auto GetFlagsForMember(const PartyMember& PMember) const -> uint16;

    // Packets
    void BroadcastPartyPackets(const CCharEntity* PSingle = nullptr);
    void ChatMessage(const ipc::ChatMessageParty& message) const;
    void ChatMessage(const ipc::ChatMessageAlliance& message) const;
    void PushPacket(uint32 senderID, uint16 ZoneID, const std::unique_ptr<CBasicPacket>& packet) const;
    void EffectsChanged();
    void PushEffectsPacket();

    // Members retrieval
    auto GetMembersWithTrusts() const -> std::vector<CBattleEntity*>;
    auto GetMembers() const -> std::vector<CCharEntity*>;
    auto GetMembers(uint16 zoneId) const -> std::vector<CCharEntity*>;
    auto GetLeader() const -> CCharEntity*;
    auto GetQuartermaster() const -> CCharEntity*;
    auto GetSyncTarget() const -> CCharEntity*;
    auto GetMemberByName(const std::string& memberName) const -> CCharEntity*;

    // Iterators
    void ForEveryMember(const std::function<void(CCharEntity*)>& func) const;
    void ForEveryMemberWithTrusts(const std::function<void(CBattleEntity*)>& func) const;
    void ForEveryAllianceMember(std::function<void(CCharEntity*)> func);

private:
    CCharParty(const ipc::PartyUpdate& message);

    void setPartyId(uint32 partyId);
    void disableSync(CCharEntity* PChar) const;
    void update(const ipc::PartyUpdate& message);
    void addMember(PartyMemberData& data);
    void delMember(const PartyMember& member);

    // This should be an array but does that really matter?
    std::vector<PartyMember>   members_;
    uint32                     m_PartyId               = 0;
    uint32                     m_LeaderUniqueNo        = 0;
    uint32                     m_QuartermasterUniqueNo = 0;
    uint32                     m_SyncTargetUniqueNo    = 0;
    timer::time_point          m_LastJoined            = timer::now();
    bool                       m_EffectsChanged        = false;
    std::unique_ptr<IpcHelper> m_pIpcHelper;

    // Allow only PartyContainer to call update() to enforce the read-only nature
    friend class PartyContainer;
};

#include "map_party_ipc_helper.h"