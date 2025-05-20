//
// Created by sruon on 5/17/2025.
//

#include "map_party.h"
#include "entities/trustentity.h"
#include "latent_effect_container.h"
#include "packets/char_status.h"
#include "packets/char_sync.h"
#include "packets/menu_config.h"
#include "packets/party_define.h"
#include "packets/party_effects.h"
#include "packets/party_member_update.h"
#include "party/flags.h"
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
        member->clearParty();
    }
}

void CCharParty::applySync(CCharEntity* PChar) const
{
    const auto* PSync = GetSyncTarget();

    if (!PSync)
    {
        return;
    }

    if (PChar->status != STATUS_TYPE::DISAPPEAR)
    {
        PChar->pushPacket<CMessageStandardPacket>(PChar->GetMLevel(), 0, 0, 0, MsgStd::LevelSyncSet);
        PChar->StatusEffectContainer->AddStatusEffect(
            new CStatusEffect(EFFECT_LEVEL_SYNC, EFFECT_LEVEL_SYNC, PSync->GetMLevel(), 0s, 0s), EffectNotice::Silent);
        PChar->StatusEffectContainer->DelStatusEffectsByFlag(EFFECTFLAG_DISPELABLE | EFFECTFLAG_ON_ZONE);
        PChar->loc.zone->PushPacket(PChar, CHAR_INRANGE, std::make_unique<CCharSyncPacket>(PChar));
    }
}

void CCharParty::disableSync(CCharEntity* PChar) const
{
    if (CStatusEffect* sync = PChar->StatusEffectContainer->GetStatusEffect(EFFECT_LEVEL_SYNC);
        sync && sync->GetDuration() == 0s)
    {
        PChar->pushPacket<CMessageBasicPacket>(PChar, PChar, 0, 30, MsgStd::LevelSyncRemoveLeftParty);
        sync->SetStartTime(timer::now());
        sync->SetDuration(30s);
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
        // clang-format off
        ForEveryMember([&](CCharEntity* PChar)
        {
            PChar->ClearTrusts();
        });
        // clang-format on
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
            // If sync target is on this server, apply sync effect to all in same zone.
            if (const auto* PSync = GetSyncTarget())
            {
                // clang-format off
                ForEveryMember([&](CCharEntity* PChar)
                {
                    if (PChar->getZone() == PSync->getZone())
                    {
                        applySync(PChar);
                    }
                });
                // clang-format on
            }
        }
        else if (m_SyncTargetUniqueNo != 0 && message.syncTargetUniqueNo == 0)
        {
            // Going from sync to no sync
            // The world server may have sent the reason as a separate message to the players
            // we are merely going to clear the sync effect.

            // clang-format off
            ForEveryMember([&](CCharEntity* PChar)
            {
                disableSync(PChar);
            });
            // clang-format on
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
                auto deletedMember = std::find_if(members_.begin(), members_.end(),
                                                  [id](const auto& member)
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

// TODO: Alliance flags
auto CCharParty::GetFlagsForMember(const CCharEntity* PChar) const -> uint16
{
    auto flags = static_cast<PartyFlag>(0);

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

// TODO: Alliance flags
auto CCharParty::GetFlagsForMember(const PartyMember& PMember) const -> uint16
{
    auto flags = static_cast<PartyFlag>(0);

    if (PMember.GetId() == m_LeaderUniqueNo)
    {
        flags = flags | PartyFlag::IsLeader;
    }

    if (PMember.GetId() == m_QuartermasterUniqueNo)
    {
        flags = flags | PartyFlag::IsQuartermaster;
    }

    if (PMember.GetId() == m_SyncTargetUniqueNo)
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

    // clang-format off
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
        // TODO: This need to work with PartyMember, not CCharEntity
        PChar->pushPacket<CPartyDefinePacket>(GetMembersWithTrusts(), m_LeaderUniqueNo, m_QuartermasterUniqueNo);
        uint8 i = 0;

        for (const auto& Member : members_)
        {
            if (Member.GetType() == PartyMemberType::Player)
            {
                if (auto* PMemberEntity = zoneutils::GetChar(Member.GetId()); PMemberEntity->getZone() == PChar->getZone())
                {
                    // If party member is on this process AND in the same zone, send a full packet
                    PChar->pushPacket<CPartyMemberUpdatePacket>(*this, PMemberEntity, i);
                }
                else
                {
                    // If party member is on a different process OR in a different zone, send a limited packet
                    PChar->pushPacket<CPartyMemberUpdatePacket>(*this, Member, i);
                }
            }
            else if (Member.GetType() == PartyMemberType::Trust)
            {
                // Trusts are special in the following ways:
                // 1. The way we build the packet is _slightly_ different
                // 2. They do not show in the party list if you're in a different zone
                // 3. They are always attached to the leader.
                // TODO: This is not how retail updates trusts but this is how LSB worked before the rewrite.

                if (const auto PLeader = GetLeader(); PLeader->getZone() == PChar->getZone())
                {
                    // PLeader is on this process and in the same zone as PChar
                    auto maybeTrust = std::find_if(PLeader->PTrusts.begin(), PLeader->PTrusts.end(),
                                                   [Member](const CTrustEntity* PTrust)
                                                   {
                                                       return PTrust->id == Member.GetId();
                                                   });
                    if (maybeTrust != PLeader->PTrusts.end())
                    {
                        PChar->pushPacket<CPartyMemberUpdatePacket>(*maybeTrust, i);
                    }
                    else
                    {
                        ShowErrorFmt("Could not find trust with ID: {} in leader's trust list?!", Member.GetId());
                    }
                }
            }

            ++i;
       }
    });
    // clang-format on
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
                auto maybeTrust = std::find_if(PLeader->PTrusts.begin(), PLeader->PTrusts.end(),
                                               [member](const CTrustEntity* PTrust)
                                               {
                                                   return PTrust->id == member.GetId();
                                               });
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
        // clang-format off
        ForEveryMember([&](CCharEntity* PChar)
        {
            PChar->pushPacket<CPartyEffectsPacket>(GetMembers());
        });
        // clang-format on
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
            PChar->setParty(*this);
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
                    applySync(PChar);
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
    const auto it = std::find_if(members_.begin(), members_.end(),
                                 [&](const PartyMember& m)
                                 { return m.GetId() == member.GetId(); });

    if (it != members_.end())
    {
        // Char may not be on this server and will be handled by another map process
        if (it->GetType() == PartyMemberType::Player)
        {
            if (CCharEntity* PChar = zoneutils::GetChar(it->GetId()))
            {
                PChar->clearParty();
                ipc().NotifyKick(member.GetId());
            }
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

void CCharParty::ChatMessage(const ipc::ChatMessageParty& message) const
{
    PushPacket(message.senderId, 0, std::make_unique<CChatMessagePacket>(message.senderName, message.zoneId, message.messageType, message.message, message.gmLevel));
}

void CCharParty::ChatMessage(const ipc::ChatMessageAlliance& message) const
{
    PushPacket(message.senderId, 0, std::make_unique<CChatMessagePacket>(message.senderName, message.zoneId, message.messageType, message.message, message.gmLevel));
}
