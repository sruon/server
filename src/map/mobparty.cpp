/*
===========================================================================

  Copyright (c) 2010-2015 Darkstar Dev Teams

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

#include "common/logging.h"
#include "common/timer.h"

#include "alliance.h"
#include "entities/battleentity.h"
#include "ipc_client.h"
#include "job_points.h"
#include "latent_effect_container.h"
#include "map_server.h"
#include "mobparty.h"
#include "status_effect_container.h"
#include "treasure_pool.h"
#include "utils/blueutils.h"
#include "utils/charutils.h"
#include "utils/jailutils.h"
#include "utils/zoneutils.h"
#include <cstring>
#include <vector>

#include "packets/char_abilities.h"
#include "packets/char_status.h"
#include "packets/char_sync.h"
#include "packets/menu_config.h"
#include "packets/message_basic.h"
#include "packets/message_standard.h"
#include "packets/party_define.h"
#include "packets/party_effects.h"
#include "packets/party_member_update.h"

// should have brace-or-equal initializers when MSVC supports it
struct CMobParty::partyInfo_t
{
    uint32      id         = {};
    uint32      partyid    = {};
    uint32      allianceid = {};
    std::string name       = {};
    uint16      flags      = {};
    uint16      zone       = {};
    uint16      prev_zone  = {};
};

// Constructor
CMobParty::CMobParty(CMobEntity* PEntity)
: m_PartyID(0)
, m_PartyType(PARTY_MOBS)
, m_PartyNumber(0)
{
    m_PLeader        = nullptr;
    m_PAlliance      = nullptr;
    m_PSyncTarget    = nullptr;
    m_PQuarterMaster = nullptr;
    m_EffectsChanged = false;

    if (PEntity != nullptr && PEntity->PParty == nullptr)
    {
        m_PartyID   = PEntity->id;
        m_PartyType = PARTY_MOBS;

        AddMember(PEntity);
        SetLeader(PEntity->name);
    }
    else
    {
        ShowWarning("CMobParty::CMobParty() - PEntity was null, or party was not null.");
    }
}

CMobParty::CMobParty(uint32 id)
: m_PartyID(id)
, m_PartyType(PARTY_PCS)
, m_PartyNumber(0)
{
    m_PAlliance = nullptr;

    m_PLeader        = nullptr;
    m_PSyncTarget    = nullptr;
    m_PQuarterMaster = nullptr;

    m_EffectsChanged = false;
}

// Dirty, ugly hack to prevent bad refs keeping garbage pointers in memory pointing to things that _could_ still be valid, causing mayhem
CMobParty::~CMobParty()
{
    m_PLeader        = nullptr;
    m_PartyID        = 0;
    m_PAlliance      = nullptr;
    m_PSyncTarget    = nullptr;
    m_PQuarterMaster = nullptr;
}

void CMobParty::DisbandParty(bool playerInitiated)
{
    m_PSyncTarget = nullptr;
    m_PLeader     = nullptr;
    m_PAlliance   = nullptr;

    if (m_PartyType == PARTY_MOBS)
    {
        for (auto& member : members) // this should really only trigger when a dynamic entity dies and nothing else qualifies for it's party anymore (such as !fafnir in zones without dragons)
        {
            member->PParty = nullptr;
        }
    }

    // TODO: This entire system needs rewriting to both:
    //     : - Make it stable
    //     : - Get rid of `delete this` and manage memory nicely
    delete this; // cpp.sh allow
}

// Assign roles to group members (players only)
void CMobParty::AssignPartyRole(const std::string& MemberName, uint8 role)
{
    if (m_PartyType != PARTY_PCS)
    {
        ShowWarning("Attempting to assign role (%d) to %s in Mob Party.", role, MemberName);
        return;
    }

    // Make sure that the the character is actually a part of this party
    const auto rset = db::preparedStmt("SELECT chars.charid FROM chars JOIN accounts_parties ON accounts_parties.charid = chars.charid WHERE charname = ? AND partyid = ?", MemberName, m_PartyID);
    if (!rset || rset->rowsCount() == 0)
    {
        return;
    }

    switch (role)
    {
        case 0:
            SetLeader(MemberName);
            break;
        case 4:
            SetQuarterMaster(MemberName);
            break;
        case 5:
            SetQuarterMaster("");
            break;
        case 6:
            SetSyncTarget(MemberName, MsgStd::LevelSyncSet);
            break;
        case 7:
            SetSyncTarget("", MsgStd::LevelSyncRemoveLeftParty);
            break;
    }

    if (m_PAlliance)
    {
        message::send(ipc::AllianceReload{
            .allianceId = m_PAlliance->m_AllianceID,
        });
    }
    else
    {
        message::send(ipc::PartyReload{
            .partyId = m_PartyID,
        });
    }
}

// get number of members in specified zone
uint8 CMobParty::MemberCount(uint16 ZoneID)
{
    uint8 count = 0;

    for (auto member : members)
    {
        if (member->getZone() == ZoneID)
        {
            count++;
        }
    }
    return count;
}

// Returns entity pointer to party member by name (used for /pcmd kick or otherwise)
CMobEntity* CMobParty::GetMemberByName(const std::string& memberName)
{
    if (m_PartyType != PARTY_PCS)
    {
        ShowWarning("Attempting to get Member data for %s in Mob Party.", memberName);
        return nullptr;
    }

    if (memberName == "")
    {
        return nullptr;
    }

    for (auto& member : members)
    {
        if (strcmpi(memberName.c_str(), member->getName().c_str()) == 0)
        {
            return member;
        }
    }

    return nullptr;
}

void CMobParty::RemoveMember(CMobEntity* PEntity)
{
    if (PEntity == nullptr || PEntity->PParty != this)
    {
        ShowWarning("CMobParty::RemoveMember() - PEntity was null, or PParty mismatch.");
        return;
    }

    if (m_PLeader == PEntity)
    {
        RemovePartyLeader(PEntity);
    }
    else
    {
        auto memberToDelete = std::find(members.begin(), members.end(), PEntity);

        if (memberToDelete != members.end())
        {
            members.erase(memberToDelete);
            PEntity->PParty = nullptr;
        }
    }
}

void CMobParty::DelMember(CMobEntity* PEntity)
{
    if (PEntity == nullptr || PEntity->PParty != this)
    {
        ShowWarning("CMobParty::DelMember() - PEntity was null, or PParty mismatch.");
        return;
    }

    if (m_PLeader == PEntity)
    {
        if (RemovePartyLeader(PEntity)) // Only reload party if party has not disbanded
        {
            this->ReloadParty();
        }
    }
    else
    {
        auto memberToDelete = std::find(members.begin(), members.end(), PEntity);

        if (memberToDelete != members.end())
        {
            PEntity->PParty = nullptr;
            members.erase(memberToDelete);
        }
        this->ReloadParty();
    }
}

void CMobParty::PopMember(CMobEntity* PEntity)
{
    if (PEntity == nullptr || PEntity->PParty != this)
    {
        ShowWarning("CMobParty::PopMember() - PEntity was null, or PParty mismatch.");
        return;
    }

    auto memberToDelete = std::find(members.begin(), members.end(), PEntity);

    if (memberToDelete != members.end())
    {
        members.erase(memberToDelete);
    }

    // free memory, party will re reinsatiated when they zone back in
    if (members.empty())
    {
        delete this; // cpp.sh allow
    }
    PEntity->PParty = nullptr;
}

bool CMobParty::RemovePartyLeader(CMobEntity* PEntity)
{
    if (members.empty())
    {
        ShowWarning("CMobParty::RemovePartyLeader - called when \"member\" list was empty");
        return false;
    }

    if (m_PartyType == PARTYTYPE::PARTY_MOBS) // mob party, mob destructor being called and is leader of a party
    {
        for (auto member : members)
        {
            if (member != PEntity) // assign leader to next party member
            {
                m_PLeader = member;
                DelMember(PEntity);

                return true;
            }
        }
    }

    if (m_PLeader == PEntity)
    {
        DisbandParty();
        return false;
    }
    else
    {
        RemoveMember(PEntity);
    }

    return true;
}

std::vector<CMobParty::partyInfo_t> CMobParty::GetPartyInfo() const
{
    std::vector<CMobParty::partyInfo_t> memberinfo;
    ShowWarning("Attempting to get Party data for Mob Party.");
    return memberinfo;
}

void CMobParty::AddMember(CMobEntity* PEntity)
{
    if (PEntity == nullptr || PEntity->PParty != nullptr)
    {
        ShowWarning("CMobParty::AddMember() - PEntity was null, or PParty not null.");
        return;
    }

    if (std::find(members.begin(), members.end(), PEntity) != members.end())
    {
        ShowWarning("CMobParty::AddMember() - PEntity was already in the member list!");
        return;
    }

    PEntity->PParty = this;
    members.emplace_back(PEntity);
}

void CMobParty::AddMember(uint32 id)
{
}

void CMobParty::PushMember(CMobEntity* PEntity)
{
    if (PEntity == nullptr || PEntity->PParty != nullptr)
    {
        ShowWarning("CMobParty::PushMember() - PEntity was null, or PParty not null.");
        return;
    }

    PEntity->PParty = this;
    members.emplace_back(PEntity);

    auto info = GetPartyInfo();

    for (auto&& memberinfo : info)
    {
        if (memberinfo.id == PEntity->id)
        {
            if (memberinfo.flags & PARTY_LEADER)
            {
                m_PLeader = PEntity;
            }
            if (memberinfo.flags & PARTY_QM)
            {
                m_PQuarterMaster = PEntity;
            }
            if (memberinfo.flags & PARTY_SYNC)
            {
                m_PSyncTarget = PEntity;
            }
        }
    }

    ReloadTreasurePool((CMobEntity*)PEntity);
}

void CMobParty::SetPartyID(uint32 id)
{
    m_PartyID = id;
}

uint32 CMobParty::GetPartyID() const
{
    return m_PartyID;
}

CMobEntity* CMobParty::GetLeader()
{
    return m_PLeader;
}

CMobEntity* CMobParty::GetSyncTarget()
{
    return m_PSyncTarget;
}

CMobEntity* CMobParty::GetQuaterMaster()
{
    return m_PQuarterMaster;
}

uint16 CMobParty::GetMemberFlags(CMobEntity* PEntity)
{
    if (PEntity == nullptr || PEntity->PParty != this)
    {
        ShowWarning("CMobParty::GetMemberFlags() - PEntity was null, or PParty mismatch.");
        return 0;
    }

    uint16 Flags = 0;

    if (PEntity->PParty->m_PartyNumber == 1)
    {
        Flags += PARTY_SECOND;
    }
    else if (PEntity->PParty->m_PartyNumber == 2)
    {
        Flags += PARTY_THIRD;
    }

    if (PEntity == m_PLeader)
    {
        Flags |= PARTY_LEADER;
    }
    if (PEntity == m_PQuarterMaster)
    {
        Flags |= PARTY_QM;
    }
    if (PEntity == m_PSyncTarget)
    {
        Flags |= PARTY_SYNC;
    }

    return Flags;
}

// update the party for all members
void CMobParty::ReloadParty()
{
    return;
}

// update party info for PChar
void CMobParty::ReloadPartyMembers(CMobEntity* PChar)
{
}

// update treasure pool for specified character
void CMobParty::ReloadTreasurePool(CMobEntity* PChar)
{
    return;
}

void CMobParty::SetLeader(const std::string& MemberName)
{
    m_PLeader = members.at(0);
}

void CMobParty::SetSyncTarget(const std::string& MemberName, MsgStd message)
{
    return;
}

// FIXME: add case for "" membername
void CMobParty::SetQuarterMaster(const std::string& MemberName)
{
    return;
}

// Send a packet to all members of the group if the zone is specified as 0
// or to the party members in the specified zone.
// Packet for PPartyMember is not sent in both cases
void CMobParty::PushPacket(uint32 senderID, uint16 ZoneID, const std::unique_ptr<CBasicPacket>& packet)
{
    return;
}

void CMobParty::PushEffectsPacket()
{
    return;
}

void CMobParty::EffectsChanged()
{
    m_EffectsChanged = true;
}

void CMobParty::DisableSync()
{
    m_PSyncTarget = nullptr;
    ReloadParty();
}

void CMobParty::RefreshSync()
{
    return;
}

void CMobParty::SetPartyNumber(uint8 number)
{
    m_PartyNumber = number;
}

bool CMobParty::HasOnlyOneMember() const
{
    if (members.size() != 1)
    {
        return false;
    }

    // Load party size to make sure that there is only one member in the party across all servers
    return LoadPartySize() == 1;
}

bool CMobParty::IsFull() const
{
    if (members.size() > 5)
    {
        return true;
    }

    // Load party size to make sure that that all members are accounted for across all servers
    return LoadPartySize() > 5;
}

uint32 CMobParty::LoadPartySize() const
{
    return static_cast<uint32>(members.size());
}

timer::time_point CMobParty::GetTimeLastMemberJoined()
{
    return timer::now();
}

bool CMobParty::HasTrusts()
{
    return false;
}

void CMobParty::RefreshFlags(std::vector<partyInfo_t>& info)
{
    // Clear pointers in case they no longer exist on this instance
    m_PLeader        = nullptr;
    m_PQuarterMaster = nullptr;
    m_PSyncTarget    = nullptr;

    for (auto&& memberinfo : info)
    {
        if (memberinfo.partyid == m_PartyID)
        {
            if (memberinfo.flags & PARTY_LEADER)
            {
                bool found = false;
                for (auto* member : members)
                {
                    if (member->id == memberinfo.id)
                    {
                        m_PLeader = member;
                        found     = true;
                    }
                }
                if (!found)
                {
                    m_PLeader = nullptr;
                }
            }
            if (memberinfo.flags & PARTY_QM)
            {
                bool found = false;
                for (auto* member : members)
                {
                    if (member->id == memberinfo.id)
                    {
                        m_PQuarterMaster = member;
                        found            = true;
                    }
                }
                if (!found)
                {
                    m_PQuarterMaster = nullptr;
                }
            }
            if (memberinfo.flags & PARTY_SYNC)
            {
                bool found = false;
                for (auto* member : members)
                {
                    if (member->id == memberinfo.id)
                    {
                        m_PSyncTarget = member;
                        found         = true;
                    }
                }
                if (!found)
                {
                    m_PSyncTarget = nullptr;
                }
            }
        }
    }
}

std::size_t CMobParty::GetMemberCountAcrossAllProcesses()
{
    // TODO: We should detect whether or not we're a multi-process
    // setup. So we can avoid asking the database for more information
    // than we need to.
    return GetPartyInfo().size();
}
