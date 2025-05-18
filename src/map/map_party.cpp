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

enum PARTYFLAG : uint16
{
    PARTY_SECOND    = 0x0001,
    PARTY_THIRD     = 0x0002,
    PARTY_LEADER    = 0x0004,
    ALLIANCE_LEADER = 0x0008,
    PARTY_QM        = 0x0010,
    PARTY_SYNC      = 0x0100,
};
DECLARE_FORMAT_AS_UNDERLYING(PARTYFLAG);

void CCharParty::update(const ipc::PartyUpdate& message)
{
    if (message.leaderUniqueNo != m_LeaderUniqueNo)
    {
        m_LeaderUniqueNo = message.leaderUniqueNo;

        // Changing leader dismisses trusts
        ForEveryMember([&](CCharEntity* PChar)
                       { PChar->ClearTrusts(); });

        // Resend the party define packet to all members
    }

    if (message.quartermasterUniqueNo != m_QuartermasterUniqueNo)
    {
        m_QuartermasterUniqueNo = message.quartermasterUniqueNo;
        // Resend the party define packet to all members
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
        // Resend the party define packet to all members
    }

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
                }
            }
        }
    }

    broadcast();
}

uint16 CCharParty::GetFlagsForMember(CCharEntity* PChar)
{
    // TODO: Alliance flags
    uint16 flags = 0;

    if (PChar->id == m_LeaderUniqueNo)
    {
        flags |= PARTY_LEADER;
    }

    if (PChar->id == m_QuartermasterUniqueNo)
    {
        flags |= PARTY_QM;
    }

    if (PChar->id == m_SyncTargetUniqueNo)
    {
        flags |= PARTY_SYNC;
    }

    return flags;
}

void CCharParty::broadcast()
{
    // Retail packet flow:
    // 0xC8: Defines party layout
    // 0xE2: Char Info with trust data
    // 0x0E: NPC update with trust
    // 0x67: Entity status
    // 0xDF: Char update with trust data
    // 0x0E: Several NPC updates with name etc
    ForEveryMember([&](CCharEntity* PChar)
                   {
                size_t memberCount = this->GetMembers().size();
                size_t trustCount  = this->GetMembersWithTrusts().size() - memberCount;

            PChar->PLatentEffectContainer->CheckLatentsPartyJobs();
            PChar->PLatentEffectContainer->CheckLatentsPartyMembers(memberCount, trustCount);
            PChar->PLatentEffectContainer->CheckLatentsPartyAvatar();
               PChar->pushPacket<CPartyDefinePacket>(GetMembersWithTrusts(), m_LeaderUniqueNo, m_QuartermasterUniqueNo);
               uint8 i = 0;
               for (auto member : GetMembersWithTrusts())
               {
                   if (auto *PCharMember = dynamic_cast<CCharEntity*>(member))
                   {
                       // TODO: This wont work cross zone
                       PChar->pushPacket<CPartyMemberUpdatePacket>(PCharMember, i, PCharMember->id == m_LeaderUniqueNo, PCharMember->id == m_QuartermasterUniqueNo);
                   }
                   else if (auto *PTrust = dynamic_cast<CTrustEntity*>(member))
                   {
                       PChar->pushPacket<CPartyMemberUpdatePacket>(PTrust, i);
                   }
                     ++i;
               } });
    //    for (auto member : message.members)
    //    {
    //        if (member.Type == PartyMemberType::Player)
    //        {
    //            CCharEntity* PChar = zoneutils::GetChar(member.UniqueNo);
    //            if (PChar)
    //            {
    //                // TODO:
    //                //PChar->PLatentEffectContainer->CheckLatentsPartyJobs();
    //                //PChar->PLatentEffectContainer->CheckLatentsPartyMembers(members.size(), trustCount);
    //                //PChar->PLatentEffectContainer->CheckLatentsPartyAvatar();
    //                //PChar->ReloadPartyDec();
    //                //auto effects = std::make_unique<CPartyEffectsPacket>();
    //                // effects->AddMemberEffects(PChar)
    //                ShowInfoFmt("Player {} ({}) is on this zone server.", PChar->getName(), PChar->id);
    //                PChar->pushPacket<CPartyDefinePacket>(message);
    //                int i = 0;
    //                for (auto otherMember : message.members)
    //                {
    //                    if (otherMember.Type == PartyMemberType::Trust)
    //                    {
    //                        CCharEntity* PLeader = zoneutils::GetChar(message.leaderUniqueNo);
    //                        if (PLeader)
    //                        {
    //                            for (auto& PTrust : PLeader->PTrusts)
    //                            {
    //                                if (PTrust->id == otherMember.UniqueNo)
    //                                {
    //                                    // Temporary: This is how it used to work in LSB but this is absolutely not how retail streams trust updates
    //                                    PChar->pushPacket<CPartyMemberUpdatePacket>(PTrust, i);
    //                                    break;
    //                                }
    //                            }
    //                        }
    //                    }
    //                    else
    //                    {
    //                        PChar->pushPacket<CPartyMemberUpdatePacket>(PChar, i, message);
    //                    }
    //                    ++i;
    //                }
    //            }
    //        }
    //    }
}
std::vector<CBattleEntity*> CCharParty::GetMembersWithTrusts() const
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

std::vector<CCharEntity*> CCharParty::GetMembers() const
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

std::vector<CCharEntity*> CCharParty::GetMembers(uint16 zoneId) const
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

CCharEntity* CCharParty::GetLeader() const
{
    // TODO: This needs to work across map processes
    return zoneutils::GetChar(m_LeaderUniqueNo);
}

CCharEntity* CCharParty::GetQuartermaster() const
{
    // TODO: This needs to work across map processes
    return zoneutils::GetChar(m_QuartermasterUniqueNo);
}

CCharEntity* CCharParty::GetSyncTarget() const
{
    // TODO: This needs to work across map processes
    return zoneutils::GetChar(m_SyncTargetUniqueNo);
}

void CCharParty::ForEveryMember(std::function<void(CCharEntity*)> func)
{
    for (auto member : GetMembers())
    {
        func(member);
    }
}

void CCharParty::ForEveryAllianceMember(std::function<void(CCharEntity*)> func)
{
}

void CCharParty::ForEveryMemberWithTrusts(std::function<void(CBattleEntity*)> func)
{
    for (auto member : GetMembersWithTrusts())
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
void CCharParty::PushPacket(uint32 senderID, uint16 ZoneID, const std::unique_ptr<CBasicPacket>& packet)
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

CCharEntity* CCharParty::GetMemberByName(const std::string& memberName)
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

void CCharParty::addMember(PartyMemberData& data)
{
    members_.push_back(data);
    m_LastJoined = timer::now();

    if (data.Type == PartyMemberType::Player)
    {
        CCharEntity* PChar = zoneutils::GetChar(data.UniqueNo);
        // Char may not be on this server and will be handled by another map process
        if (PChar)
        {
            PChar->PParty = this;
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
            if (auto* PSync = GetSyncTarget())
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
    auto it = std::find_if(members_.begin(), members_.end(), [&](const PartyMember& m)
                           { return m.GetId() == member.GetId(); });

    if (it != members_.end())
    {
        ipc().NotifyKick(member.GetId());
        members_.erase(it);
    }
}

CCharParty::CCharParty(const ipc::PartyUpdate& message)
{
    m_pIpcHelper            = IpcHelper::Create(*this);
    m_PartyId               = message.partyId;
    m_LeaderUniqueNo        = message.leaderUniqueNo;
    m_QuartermasterUniqueNo = message.quartermasterUniqueNo;
    m_SyncTargetUniqueNo    = message.syncTargetUniqueNo;

    // Defer to AddMember or something
    for (auto member : message.members)
    {
        addMember(member);
    }

    broadcast();
}
const CCharParty::IpcHelper& CCharParty::ipc() const
{
    return *m_pIpcHelper;
}

bool CCharParty::IsFull() const
{
    return members_.size() >= 6;
}

bool CCharParty::HasOnlyOneMember() const
{
    return members_.size() == 1;
}

timer::time_point CCharParty::GetTimeLastMemberJoined() const
{
    return m_LastJoined;
}

bool CCharParty::HasTrusts()
{
    return std::find_if(members_.begin(), members_.end(), [](auto& member)
                        { return member.GetType() == PartyMemberType::Trust; }) != members_.end();
}