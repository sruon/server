//
// Created by sruon on 5/17/2025.
//

#include "map_party.h"
#include "latent_effect_container.h"
#include "packets/char_status.h"
#include "packets/char_sync.h"
#include "packets/menu_config.h"
#include "status_effect_container.h"
#include "utils/charutils.h"
#include "utils/jailutils.h"
#include "utils/zoneutils.h"

CCharParty::CCharParty(const ipc::PartyUpdate& message)
{
    m_pIpcHelper            = IpcHelper::Create(*this);
    m_PartyId               = message.partyId;
    m_LeaderUniqueNo        = message.leaderUniqueNo;
    m_QuartermasterUniqueNo = message.quartermasterUniqueNo;
    m_SyncTargetUniqueNo    = message.syncTargetUniqueNo;

    for (auto member : message.members)
    {
        addMember(member);
    }

    BroadcastPartyPackets();
}

// TODO: Not sure we need this logic since the container only deletes a party when all members are gone
CCharParty::~CCharParty()
{
    for (const auto member : GetMembers())
    {
        ShowErrorFmt("CCharParty destructor called with members.");
        member->ClearParty();
    }
}

// Receives party updates from the world server
// Determines changes, if any, and updates the party.
// This may trigger additional IPC messages.
void CCharParty::update(const ipc::PartyUpdate& message)
{
    bool changes = false;

    if (message.leaderUniqueNo != m_LeaderUniqueNo)
    {
        m_LeaderUniqueNo = message.leaderUniqueNo;

        // Changing leader dismisses trusts
        ForEveryMember([&](CCharEntity* PChar)
                       { PChar->ClearTrusts(); });
        changes = true;
    }

    if (message.quartermasterUniqueNo != m_QuartermasterUniqueNo)
    {
        m_QuartermasterUniqueNo = message.quartermasterUniqueNo;
        changes                 = true;
    }

    if (message.syncTargetUniqueNo != m_SyncTargetUniqueNo)
    {
        if (m_SyncTargetUniqueNo == 0 && message.syncTargetUniqueNo != 0)
        {
            // Going from no sync to sync
        }
        else if (m_SyncTargetUniqueNo != 0 && message.syncTargetUniqueNo == 0)
        {
            // Going from sync to no sync
            ForEveryMember([&](CCharEntity* PChar)
                           {
                CStatusEffect* sync = PChar->StatusEffectContainer->GetStatusEffect(EFFECT_LEVEL_SYNC);
                if (sync && sync->GetDuration() == 0s)
                {
                    PChar->pushPacket<CMessageBasicPacket>(PChar, PChar, 0, 30, MsgStd::LevelSyncRemoveLeftParty);
                    sync->SetStartTime(timer::now());
                    sync->SetDuration(30s);
                } });
        }

        m_SyncTargetUniqueNo = message.syncTargetUniqueNo;
        changes              = true;
    }

    // This compares party membership.
    // TODO: Deeper checks, including zone IDs.
    if (message.members.size() != members_.size())
    {
        std::unordered_set<uint32> oldIds, newIds;

        // Extract old IDs
        for (const auto& member : members_)
        {
            oldIds.insert(member.GetId());
        }

        // Extract new IDs
        for (const auto& member : message.members)
        {
            newIds.insert(member.UniqueNo);
        }

        // Find added IDs (in new but not in old)
        for (const uint32 id : newIds)
        {
            if (oldIds.find(id) == oldIds.end())
            {
                for (auto member : message.members)
                {
                    if (member.UniqueNo == id)
                    {
                        addMember(member);
                        changes = true;
                        break;
                    }
                }
            }
        }

        // Find deleted IDs (in old but not in new)
        for (const uint32 id : oldIds)
        {
            if (newIds.find(id) == newIds.end())
            {
                auto deletedMember = std::find_if(members_.begin(), members_.end(), [id](const auto& member)
                                                  { return member.GetId() == id; });
                if (deletedMember != members_.end())
                {
                    ShowInfoFmt("Removing member with ID: {}", deletedMember->GetId());
                    delMember(*deletedMember);
                    changes = true;
                }
            }
        }
    }

    if (changes)
    {
        BroadcastPartyPackets();
    }
}

auto CCharParty::GetFlagsForMember(const CCharEntity* PChar) const -> uint16
{
    auto flags = static_cast<PartyFlag>(0);

    if (!PChar)
    {
        return static_cast<uint16>(flags);
    }

    if (PChar->id == m_LeaderUniqueNo)
    {
        flags = flags | PartyFlag::IsLeader;
    }

    if (PChar->id == m_QuartermasterUniqueNo)
    {
        flags = flags | PartyFlag::IsQuartermaster;
    }

    if (PChar->id == m_SyncTargetUniqueNo)
    {
        flags = flags | PartyFlag::IsSyncTarget;
    }

    return static_cast<uint16>(flags);
}

// Recalculate latents, send party define and party update packets.
// Can optionally be scoped to a single entity.
void CCharParty::BroadcastPartyPackets(const CCharEntity* PSingle)
{
    // Retail packet flow:
    // 0xC8: Defines party layout
    // 0xE2: Char Info with trust data
    // 0x0E: NPC update with trust
    // 0x67: Entity status
    // 0xDF: Char update with trust data
    // 0x0E: Several NPC updates with name etc
    const size_t memberCount = this->GetMembers().size();
    const size_t trustCount  = this->GetMembersWithTrusts().size() - memberCount;

    //clang-format off
    ForEveryMember([&](CCharEntity* PChar)
                   {
        if (PSingle != nullptr && PChar != PSingle)
        {
            return;
        }

        // TODO: Party effects
        // auto effects = std::make_unique<CPartyEffectsPacket>();
        // effects->AddMemberEffects(PChar)
        PChar->PLatentEffectContainer->CheckLatentsPartyJobs();
        PChar->PLatentEffectContainer->CheckLatentsPartyMembers(memberCount, trustCount);
        PChar->PLatentEffectContainer->CheckLatentsPartyAvatar();
        PChar->pushPacket<CPartyDefinePacket>(GetMembersWithTrusts(), m_LeaderUniqueNo, m_QuartermasterUniqueNo);
        uint8 i = 0;

        for (const auto PMember : GetMembersWithTrusts())
        {
            if (auto *PCharMember = dynamic_cast<CCharEntity*>(PMember))
            {
                // TODO: This wont work cross zone
                PChar->pushPacket<CPartyMemberUpdatePacket>(PCharMember, i, PCharMember->id == m_LeaderUniqueNo, PCharMember->id == m_QuartermasterUniqueNo);
            }
            else if (auto *PTrust = dynamic_cast<CTrustEntity*>(PMember))
            {
                // This is wrong for trusts but that's how it used to work before.
                PChar->pushPacket<CPartyMemberUpdatePacket>(PTrust, i);
            }

            ++i;
       } });
    //clang-format on
}

// Returns a vector of CCharEntity present on this map server, along with the trusts.
// This is not guaranteed to be the full set of party members.
auto CCharParty::GetMembersWithTrusts() const -> std::vector<CBattleEntity*>
{
    auto  result  = std::vector<CBattleEntity*>{};
    auto* PLeader = GetLeader();

    for (auto member : members_)
    {
        if (member.GetType() == PartyMemberType::Player)
        {
            if (auto* PChar = zoneutils::GetChar(member.GetId()))
            {
                result.push_back(PChar);
            }
        }
        else if (member.GetType() == PartyMemberType::Trust)
        {
            if (PLeader)
            {
                auto maybeTrust = std::find_if(PLeader->PTrusts.begin(), PLeader->PTrusts.end(), [member](const CTrustEntity* PTrust)
                                               { return PTrust->id == member.GetId(); });
                if (maybeTrust != PLeader->PTrusts.end())
                {
                    result.push_back(*maybeTrust);
                }
            }
        }
    }

    return result;
}

// Returns a vector of CCharEntity present on this map server
// This is not guaranteed to be the full set of party members.
auto CCharParty::GetMembers() const -> std::vector<CCharEntity*>
{
    auto result = std::vector<CCharEntity*>{};

    for (auto member : members_)
    {
        if (auto* PChar = zoneutils::GetChar(member.GetId()))
        {
            result.push_back(PChar);
        }
    }

    return result;
}

// Returns a vector of CCharEntity present on this map server in the given zone.
// This is not guaranteed to be the full set of party members.
auto CCharParty::GetMembers(const uint16 zoneId) const -> std::vector<CCharEntity*>
{
    auto result = std::vector<CCharEntity*>{};

    for (auto member : members_)
    {
        if (auto* PChar = zoneutils::GetChar(member.GetId()))
        {
            if (PChar->getZone() == zoneId)
            {
                result.push_back(PChar);
            }
        }
    }

    return result;
}

// Returns the entity representing the party leader
// This only works if the leader is on the same map process
auto CCharParty::GetLeader() const -> CCharEntity*
{
    // TODO: This needs to work across map processes
    return zoneutils::GetChar(m_LeaderUniqueNo);
}

// Returns the entity representing the quartermaster
// This only works if the quartermaster is on the same map process
auto CCharParty::GetQuartermaster() const -> CCharEntity*
{
    // TODO: This needs to work across map processes
    return zoneutils::GetChar(m_QuartermasterUniqueNo);
}

// Returns the entity representing the sync target
// This only works if the sync target is on the same map process
auto CCharParty::GetSyncTarget() const -> CCharEntity*
{
    // TODO: This needs to work across map processes
    return zoneutils::GetChar(m_SyncTargetUniqueNo);
}

// Executes an arbitrary function for each party member present on this map process
void CCharParty::ForEveryMember(const std::function<void(CCharEntity*)>& func) const
{
    for (const auto member : GetMembers())
    {
        func(member);
    }
}

// Executes an arbitrary function for each alliance member present on this map process
void CCharParty::ForEveryAllianceMember(std::function<void(CCharEntity*)> func)
{
}

// Executes an arbitrary function for each party member present on this map process, including the trusts.
void CCharParty::ForEveryMemberWithTrusts(const std::function<void(CBattleEntity*)>& func) const
{
    for (const auto member : GetMembersWithTrusts())
    {
        func(member);
    }
}

void CCharParty::PushEffectsPacket()
{
    if (m_EffectsChanged)
    {
        ForEveryMember([&](CCharEntity* PChar)
                       { PChar->pushPacket<CPartyEffectsPacket>(GetMembers()); });

        m_EffectsChanged = false;
    }
}

// Send a packet to all members of the group if the zone is specified as 0
// or to the party members in the specified zone.
// Packet for PPartyMember is not sent in both cases
void CCharParty::PushPacket(uint32 senderID, uint16 ZoneID, const std::unique_ptr<CBasicPacket>& packet) const
{
    for (auto& member : GetMembers())
    {
        if (member->id != senderID && member->status != STATUS_TYPE::DISAPPEAR && !jailutils::InPrison(member))
        {
            if (ZoneID == 0 || member->getZone() == ZoneID)
            {
                member->pushPacket(packet->copy());
            }
        }
    }
}

auto CCharParty::GetMemberByName(const std::string& memberName) const -> CCharEntity*
{
    for (auto& member : GetMembers())
    {
        if (member->getName() == memberName)
        {
            return member;
        }
    }
    return nullptr;
}

auto CCharParty::GetPartyId() const -> uint32
{
    return m_PartyId;
}

void CCharParty::EffectsChanged()
{
    m_EffectsChanged = true;
}

void CCharParty::setPartyId(const uint32 partyId)
{
    m_PartyId = partyId;
}

void CCharParty::addMember(PartyMemberData& data)
{
    members_.emplace_back(data);
    m_LastJoined = timer::now();

    if (data.Type == PartyMemberType::Player)
    {
        // Char may not be on this server and will be handled by another map process
        if (CCharEntity* PChar = zoneutils::GetChar(data.UniqueNo))
        {
            PChar->SetParty(*this);
            // this is garbage and should be handled elsewhere
            //                        ReloadTreasurePool(PChar);

            if (PChar->isSeekingParty())
            {
                PChar->playerConfig.InviteFlg = false;
                PChar->updatemask |= UPDATE_HP;

                charutils::SaveCharStats(PChar);
                charutils::SavePlayerSettings(PChar);

                PChar->pushPacket<CMenuConfigPacket>(PChar);
                PChar->pushPacket<CCharStatusPacket>(PChar);
                PChar->pushPacket<CCharSyncPacket>(PChar);
            }

            PChar->PTreasurePool->updatePool(PChar);

            // Apply level sync if the party is level synced
            if (const auto* PSync = GetSyncTarget())
            {
                if (PChar->getZone() == PSync->getZone())
                {
                    PChar->pushPacket<CMessageBasicPacket>(PChar, PChar, 0, PSync->GetMLevel(), MsgStd::LevelSyncActivated);
                    PChar->StatusEffectContainer->AddStatusEffect(new CStatusEffect(EFFECT_LEVEL_SYNC, EFFECT_LEVEL_SYNC, PSync->GetMLevel(), 0s, 0s), EffectNotice::Silent);
                    PChar->StatusEffectContainer->DelStatusEffectsByFlag(EFFECTFLAG_DISPELABLE | EFFECTFLAG_ON_ZONE);
                    PChar->loc.zone->PushPacket(PChar, CHAR_INRANGE, std::make_unique<CCharSyncPacket>(PChar));
                }
            }

            // You lose all your summoned trusts upon joining a party
            PChar->ClearTrusts();

            PChar->m_charHistory.joinedParties++;
        }
    }
}

void CCharParty::delMember(const PartyMember& member)
{
    const auto it = std::find_if(members_.begin(), members_.end(), [&](const PartyMember& m)
                                 { return m.GetId() == member.GetId(); });

    if (it != members_.end())
    {
        // Char may not be on this server and will be handled by another map process
        if (it->GetType() == PartyMemberType::Player)
        {
            if (CCharEntity* PChar = zoneutils::GetChar(it->GetId()))
            {
                PChar->ClearParty();
            }

            ipc().NotifyKick(member.GetId());
        }

        // but we still remove it from our list!
        members_.erase(it);
    }
}

const CCharParty::IpcHelper& CCharParty::ipc() const
{
    return *m_pIpcHelper;
}

// The world server should decide if we're full, but this is an easy helper.
auto CCharParty::IsFull() const -> bool
{
    return members_.size() >= 6;
}

auto CCharParty::HasOnlyOneMember() const -> bool
{
    return members_.size() == 1;
}

auto CCharParty::GetTimeLastMemberJoined() const -> timer::time_point
{
    return m_LastJoined;
}

bool CCharParty::HasTrusts()
{
    return std::find_if(members_.begin(), members_.end(), [](auto& member)
                        { return member.GetType() == PartyMemberType::Trust; }) != members_.end();
}

// Returns true if the party only contains the leader and their trusts.
//
// There are certain conditions where packets are processed differently
// if the player is in a "fake" party only with trusts.
auto CCharParty::IsTrustOnlyParty() const -> bool
{
    for (auto& member : members_)
    {
        if (member.GetId() == m_LeaderUniqueNo)
        {
            continue;
        }

        if (member.GetType() == PartyMemberType::Player)
        {
            return false;
        }
    }

    return true;
}

auto CCharParty::ChatMessage(const ipc::ChatMessageParty& message) -> bool
{
    PushPacket(message.senderId, 0, std::make_unique<CChatMessagePacket>(message.senderName, message.zoneId, message.messageType, message.message, message.gmLevel));
    return true;
}

auto CCharParty::ChatMessage(const ipc::ChatMessageAlliance& message) -> bool
{
    PushPacket(message.senderId, 0, std::make_unique<CChatMessagePacket>(message.senderName, message.zoneId, message.messageType, message.message, message.gmLevel));
    return true;
}