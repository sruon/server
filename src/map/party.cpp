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
#include "entities/trustentity.h"
#include "ipc_client.h"
#include "job_points.h"
#include "latent_effect_container.h"
#include "map_server.h"
#include "party.h"
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
struct CParty::partyInfo_t
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
CParty::CParty(CBattleEntity* PEntity)
: m_PartyID(0)
, m_PartyType(PARTY_PCS)
, m_PartyNumber(0)
{
    m_PLeader        = nullptr;
    m_PAlliance      = nullptr;
    m_PSyncTarget    = nullptr;
    m_PQuarterMaster = nullptr;
    m_EffectsChanged = false;

    if (const auto* PChar = dynamic_cast<CCharEntity*>(PEntity); PChar && !PChar->HasParty())
    {
        m_PartyID   = PEntity->id;
        m_PartyType = PARTY_PCS;

        message::send(ipc::PartyCreate{
            .charId = PEntity->id,
            .zoneId = PEntity->getZone() });

        message::send(ipc::PartyAddMember{
            .partyId = this->GetPartyID(),
            .charId  = PEntity->id,
            .type    = PartyMemberType::Player,
            .zoneId  = PEntity->getZone(),
        });
        // AddMember(PEntity);
        // SetLeader(PEntity->name);
    }
    else
    {
        ShowWarning("CParty::CParty() - PEntity was null, or party was not null.");
    }
}

CParty::CParty(uint32 id)
: m_PartyID(id)
, m_PartyType(PARTY_PCS)
, m_PartyNumber(0)
{
    m_PAlliance = nullptr;

    m_PLeader        = nullptr;
    m_PSyncTarget    = nullptr;
    m_PQuarterMaster = nullptr;

    m_EffectsChanged = false;
    // This is only used to create a party that already exists in another zone. Not relevant anymore
}

// Dirty, ugly hack to prevent bad refs keeping garbage pointers in memory pointing to things that _could_ still be valid, causing mayhem
CParty::~CParty()
{
    m_PLeader        = nullptr;
    m_PartyID        = 0;
    m_PAlliance      = nullptr;
    m_PSyncTarget    = nullptr;
    m_PQuarterMaster = nullptr;
}

// Assign roles to group members (players only)
void CParty::AssignPartyRole(const std::string& MemberName, uint8 role)
{
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
            // message::send(ipc::PartySetLeader{
            //     .partyId = this->GetPartyID(),
            //     .charId  = GetMemberByName(MemberName)->id,
            // });
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
        // message::send(ipc::AllianceReload{
        //     .allianceId = m_PAlliance->m_AllianceID,
        // });
    }
    else
    {
        // message::send(ipc::PartyReload{
        //     .partyId = m_PartyID,
        // });
    }
}

// Returns entity pointer to party member by name (used for /pcmd kick or otherwise)
CBattleEntity* CParty::GetMemberByName(const std::string& memberName)
{
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

// void CParty::RemoveMember(CBattleEntity* PEntity)
// {
//     message::send(ipc::PartyRemoveMember{
//         .partyId = this->GetPartyID(),
//         .charId  = PEntity->id,
//     });
//     if (PEntity == nullptr || static_cast<CCharEntity*>(PEntity)->PParty != this)
//     {
//         ShowWarning("CParty::RemoveMember() - PEntity was null, or PParty mismatch.");
//         return;
//     }
//
//     if (m_PLeader == PEntity)
//     {
//         RemovePartyLeader(PEntity);
//
//         // Remove their trusts
//         CCharEntity* PChar = dynamic_cast<CCharEntity*>(PEntity);
//         if (PChar)
//         {
//             PChar->ClearTrusts();
//         }
//     }
//     else
//     {
//         auto memberToDelete = std::find(members.begin(), members.end(), PEntity);
//
//         if (memberToDelete != members.end())
//         {
//             if (m_PartyType == PARTY_PCS && PEntity->objtype == TYPE_PC)
//             {
//                 CCharEntity* PChar = static_cast<CCharEntity*>(PEntity);
//
//                 if (m_PQuarterMaster == PChar)
//                 {
//                     SetQuarterMaster("");
//                 }
//                 if (m_PSyncTarget == PChar)
//                 {
//                     SetSyncTarget("", MsgStd::LevelSyncRemoveLeftParty);
//                     CStatusEffect* sync = PChar->StatusEffectContainer->GetStatusEffect(EFFECT_LEVEL_SYNC);
//                     if (sync && sync->GetDuration() == 0s)
//                     {
//                         PChar->pushPacket<CMessageBasicPacket>(PChar, PChar, 0, 30, MsgStd::LevelSyncRemoveLeftParty);
//                         sync->SetStartTime(timer::now());
//                         sync->SetDuration(30s);
//                     }
//                     DisableSync();
//                 }
//                 if (m_PSyncTarget != nullptr && m_PSyncTarget != PChar)
//                 {
//                     if (PChar->status != STATUS_TYPE::DISAPPEAR)
//                     {
//                         CStatusEffect* sync = PChar->StatusEffectContainer->GetStatusEffect(EFFECT_LEVEL_SYNC);
//                         if (sync && sync->GetDuration() == 0s)
//                         {
//                             PChar->pushPacket<CMessageBasicPacket>(PChar, PChar, 0, 30, MsgStd::LevelSyncRemoveLeftParty);
//                             sync->SetStartTime(timer::now());
//                             sync->SetDuration(30s);
//                         }
//                     }
//                 }
//
//                 size_t trustCount = 0;
//                 if (m_PLeader != nullptr)
//                 {
//                     trustCount = static_cast<CCharEntity*>(m_PLeader)->PTrusts.size();
//                 }
//
//                 PChar->PLatentEffectContainer->CheckLatentsPartyMembers(members.size(), trustCount);
//
//                 // PChar->pushPacket<CPartyDefinePacket>(nullptr);
//                 //  PChar->pushPacket<CPartyMemberUpdatePacket>(PChar, 0, 0, PChar->getZone());
//                 PChar->pushPacket<CCharStatusPacket>(PChar);
//
//                 db::preparedStmt("DELETE FROM accounts_parties WHERE charid = ?", PChar->id);
//
//                 if (m_PAlliance)
//                 {
//                     message::send(ipc::AllianceReload{
//                         .allianceId = m_PAlliance->m_AllianceID,
//                     });
//                 }
//                 else
//                 {
//                     message::send(ipc::PartyReload{
//                         .partyId = m_PartyID,
//                     });
//                 }
//
//                 if (PChar->PTreasurePool != nullptr && PChar->PTreasurePool->getPoolType() != TreasurePoolType::Zone)
//                 {
//                     PChar->PTreasurePool->delMember(PChar);
//                     PChar->PTreasurePool = new CTreasurePool(TreasurePoolType::Solo);
//                     PChar->PTreasurePool->addMember(PChar);
//                     PChar->PTreasurePool->updatePool(PChar);
//                 }
//             }
//
//             members.erase(memberToDelete);
//
//             static_cast<CCharEntity*>(PEntity)->PParty = nullptr;
//         }
//     }
// }
//
// void CParty::DelMember(CBattleEntity* PEntity)
// {
//     if (PEntity == nullptr || static_cast<CCharEntity*>(PEntity)->PParty != this)
//     {
//         ShowWarning("CParty::DelMember() - PEntity was null, or PParty mismatch.");
//         return;
//     }
//
//     if (m_PLeader == PEntity)
//     {
//         if (RemovePartyLeader(PEntity)) // Only reload party if party has not disbanded
//         {
//             // this->ReloadParty();
//         }
//     }
//     else
//     {
//         auto memberToDelete = std::find(members.begin(), members.end(), PEntity);
//
//         if (memberToDelete != members.end())
//         {
//             if (m_PartyType == PARTY_PCS && PEntity->objtype == TYPE_PC)
//             {
//                 CCharEntity* PChar = static_cast<CCharEntity*>(PEntity);
//
//                 if (m_PQuarterMaster == PChar)
//                 {
//                     SetQuarterMaster("");
//                 }
//                 if (m_PSyncTarget == PChar)
//                 {
//                     SetSyncTarget("", MsgStd::LevelSyncRemoveLeftParty);
//                     CStatusEffect* sync = PChar->StatusEffectContainer->GetStatusEffect(EFFECT_LEVEL_SYNC);
//                     if (sync && sync->GetDuration() == 0s)
//                     {
//                         PChar->pushPacket<CMessageBasicPacket>(PChar, PChar, 0, 30, MsgStd::LevelSyncRemoveLeftParty);
//                         sync->SetStartTime(timer::now());
//                         sync->SetDuration(30s);
//                     }
//                     DisableSync();
//                 }
//                 if (m_PSyncTarget != nullptr && m_PSyncTarget != PChar)
//                 {
//                     if (PChar->status != STATUS_TYPE::DISAPPEAR)
//                     {
//                         CStatusEffect* sync = PChar->StatusEffectContainer->GetStatusEffect(EFFECT_LEVEL_SYNC);
//                         if (sync && sync->GetDuration() == 0s)
//                         {
//                             PChar->pushPacket<CMessageBasicPacket>(PChar, PChar, 0, 30, MsgStd::LevelSyncRemoveLeftParty);
//                             sync->SetStartTime(timer::now());
//                             sync->SetDuration(30s);
//                         }
//                     }
//                 }
//                 PChar->PLatentEffectContainer->CheckLatentsPartyMembers(members.size(), 0);
//
//                 // PChar->pushPacket<CPartyDefinePacket>(nullptr);
//                 //  PChar->pushPacket<CPartyMemberUpdatePacket>(PChar, 0, 0, PChar->getZone());
//                 PChar->pushPacket<CCharStatusPacket>(PChar);
//                 PChar->PParty = nullptr;
//
//                 if (PChar->PTreasurePool != nullptr && PChar->PTreasurePool->getPoolType() != TreasurePoolType::Zone)
//                 {
//                     PChar->PTreasurePool->delMember(PChar);
//                     PChar->PTreasurePool = new CTreasurePool(TreasurePoolType::Solo);
//                     PChar->PTreasurePool->addMember(PChar);
//                     PChar->PTreasurePool->updatePool(PChar);
//                 }
//             }
//             members.erase(memberToDelete);
//         }
//         // this->ReloadParty();
//     }
// }
//
// void CParty::PopMember(CBattleEntity* PEntity)
// {
//     if (PEntity == nullptr || static_cast<CCharEntity*>(PEntity)->PParty != this)
//     {
//         ShowWarning("CParty::PopMember() - PEntity was null, or PParty mismatch.");
//         return;
//     }
//
//     auto memberToDelete = std::find(members.begin(), members.end(), PEntity);
//
//     if (memberToDelete != members.end())
//     {
//         members.erase(memberToDelete);
//     }
//
//     // free memory, party will re reinsatiated when they zone back in
//     if (members.empty())
//     {
//         if (m_PAlliance)
//         {
//             if (m_PAlliance->getMainParty() == this)
//             {
//                 m_PAlliance->setMainParty(nullptr);
//             }
//
//             auto it = m_PAlliance->partyList.begin();
//             while (it != m_PAlliance->partyList.end())
//             {
//                 if (this == *it)
//                 {
//                     it = m_PAlliance->partyList.erase(it);
//                     continue;
//                 }
//                 it++;
//             }
//         }
//         delete this; // cpp.sh allow
//     }
//     static_cast<CCharEntity*>(PEntity)->PParty = nullptr;
// }

bool CParty::RemovePartyLeader(CBattleEntity* PEntity)
{
    if (members.empty())
    {
        ShowWarning("CParty::RemovePartyLeader - called when \"member\" list was empty");
        return false;
    }

    if (m_PartyType != PARTYTYPE::PARTY_MOBS)
    {
        const auto rset = db::preparedStmt("SELECT charname FROM accounts_sessions JOIN chars ON accounts_sessions.charid = chars.charid "
                                           "JOIN accounts_parties ON accounts_parties.charid = chars.charid WHERE partyid = ? AND NOT partyflag & ? "
                                           "ORDER BY timestamp ASC LIMIT 1",
                                           m_PartyID, PARTY_LEADER);
        if (rset && rset->rowsCount() && rset->next())
        {
            std::string newLeader = rset->get<std::string>("charname");
            SetLeader(newLeader);
        }
    }

    if (m_PLeader == PEntity)
    {
        // DisbandParty();
        return false;
    }
    // else
    // {
    //     RemoveMember(PEntity);
    // }

    return true;
}

std::vector<CParty::partyInfo_t> CParty::GetPartyInfo() const
{
    std::vector<CParty::partyInfo_t> memberinfo;

    if (m_PartyType != PARTY_PCS)
    {
        ShowWarning("Attempting to get Party data for Mob Party.");
        return memberinfo;
    }

    const auto rset = db::preparedStmt("SELECT chars.charid, partyid, allianceid, charname, partyflag, pos_zone, pos_prevzone FROM accounts_parties "
                                       "LEFT JOIN chars ON accounts_parties.charid = chars.charid WHERE "
                                       "(allianceid <> 0 AND allianceid = ?) OR partyid = ? ORDER BY partyflag & ?, timestamp",
                                       m_PAlliance ? m_PAlliance->m_AllianceID : 0, m_PartyID, PARTY_SECOND | PARTY_THIRD);
    if (rset && rset->rowsCount())
    {
        while (rset->next())
        {
            memberinfo.emplace_back(CParty::partyInfo_t{
                .id         = rset->get<uint32>("charid"),
                .partyid    = rset->get<uint32>("partyid"),
                .allianceid = rset->get<uint32>("allianceid"),
                .name       = rset->get<std::string>("charname"),
                .flags      = rset->get<uint16>("partyflag"),
                .zone       = rset->get<uint16>("pos_zone"),
                .prev_zone  = rset->get<uint16>("pos_prevzone"),
            });
        }
    }

    return memberinfo;
}

void CParty::SetPartyID(uint32 id)
{
    m_PartyID = id;
}

uint32 CParty::GetPartyID() const
{
    return m_PartyID;
}

CBattleEntity* CParty::GetLeader()
{
    return m_PLeader;
}

CBattleEntity* CParty::GetSyncTarget()
{
    return m_PSyncTarget;
}

CBattleEntity* CParty::GetQuaterMaster()
{
    return m_PQuarterMaster;
}

// update treasure pool for specified character
// void CParty::ReloadTreasurePool(CCharEntity* PChar)
// {
//     if (PChar == nullptr)
//     {
//         ShowWarning("CParty::ReloadTreasurePool() - PChar was null.");
//         return;
//     }
//
//     if (PChar->PTreasurePool != nullptr && PChar->PTreasurePool->getPoolType() == TreasurePoolType::Zone)
//     {
//         return;
//     }
//
//     // alliance
//     if (PChar->PParty != nullptr)
//     {
//         if (PChar->PParty->m_PAlliance != nullptr)
//         {
//             for (std::size_t a = 0; a < PChar->PParty->m_PAlliance->partyList.size(); ++a)
//             {
//                 for (std::size_t i = 0; i < PChar->PParty->m_PAlliance->partyList.at(a)->members.size(); ++i)
//                 {
//                     CCharEntity* PPartyMember = (CCharEntity*)PChar->PParty->m_PAlliance->partyList.at(a)->members.at(i);
//
//                     if (PPartyMember != PChar && PPartyMember->PTreasurePool != nullptr && PPartyMember->getZone() == PChar->getZone())
//                     {
//                         if (PChar->PTreasurePool != nullptr)
//                         {
//                             PChar->PTreasurePool->delMember(PChar);
//                         }
//                         PChar->PTreasurePool = PPartyMember->PTreasurePool;
//                         PChar->PTreasurePool->addMember(PChar);
//                         return;
//                     }
//                 }
//
//             } // regular party
//         }
//         else if (PChar->PParty->m_PAlliance == nullptr)
//         {
//             for (auto& member : members)
//             {
//                 CCharEntity* PPartyMember = (CCharEntity*)member;
//
//                 if (PPartyMember != PChar && PPartyMember->PTreasurePool != nullptr && PPartyMember->getZone() == PChar->getZone())
//                 {
//                     if (PChar->PTreasurePool != nullptr)
//                     {
//                         PChar->PTreasurePool->delMember(PChar);
//                     }
//                     PChar->PTreasurePool = PPartyMember->PTreasurePool;
//                     PChar->PTreasurePool->addMember(PChar);
//                     return;
//                 }
//             }
//         }
//     }
//
//     if (PChar->PTreasurePool == nullptr)
//     {
//         PChar->PTreasurePool = new CTreasurePool(TreasurePoolType::Solo);
//         PChar->PTreasurePool->addMember(PChar);
//     }
// }

void CParty::SetLeader(const std::string& MemberName)
{
    if (m_PartyType == PARTY_PCS)
    {
        uint32 newId = 0;

        const auto rset = db::preparedStmt("SELECT chars.charid from accounts_sessions JOIN chars ON chars.charid = accounts_sessions.charid WHERE charname = ?", MemberName);
        if (rset && rset->rowsCount() && rset->next())
        {
            newId = rset->get<uint32>(0);
        }
        else
        {
            return;
        }

        db::preparedStmt("UPDATE accounts_parties SET partyflag = partyflag & ~? WHERE partyid = ? AND partyflag & ?", ALLIANCE_LEADER | PARTY_LEADER, m_PartyID, PARTY_LEADER);
        db::preparedStmt("UPDATE accounts_parties SET partyid = ? WHERE partyid = ?", newId, m_PartyID);
        db::preparedStmt("UPDATE accounts_parties SET allianceid = ? WHERE allianceid = ?", newId, m_PartyID);

        m_PLeader = GetMemberByName(MemberName);
        if (this->m_PAlliance && this->m_PAlliance->m_AllianceID == m_PartyID)
        {
            m_PAlliance->m_AllianceID = newId;
        }
        // message::send(ipc::PartySetLeader{
        //     .partyId = this->GetPartyID(),
        //     .charId  = m_PLeader->id,
        // });
        m_PartyID = newId;
        db::preparedStmt("UPDATE accounts_parties SET partyflag = partyflag | IF(allianceid = partyid, ?, ?) WHERE charid = ?", ALLIANCE_LEADER | PARTY_LEADER, PARTY_LEADER, newId);

        // Passing leader dismisses trusts
    }
    else
    {
        m_PLeader = members.at(0);
    }
}

void CParty::SetSyncTarget(const std::string& MemberName, MsgStd message)
{
    CBattleEntity* PEntity = GetMemberByName(MemberName);

    if (settings::get<bool>("map.LEVEL_SYNC_ENABLE"))
    {
        if (PEntity && PEntity->objtype == TYPE_PC)
        {
            CCharEntity* PChar = (CCharEntity*)PEntity;
            // enable level sync
            if (PChar->GetMLevel() < 10)
            {
                ((CCharEntity*)GetLeader())->pushPacket<CMessageBasicPacket>((CCharEntity*)GetLeader(), (CCharEntity*)GetLeader(), 0, 10, MsgStd::LevelSyncDesigneeBelowMin);
                return;
            }
            else if (PChar->getZone() != GetLeader()->getZone())
            {
                ((CCharEntity*)GetLeader())->pushPacket<CMessageBasicPacket>((CCharEntity*)GetLeader(), (CCharEntity*)GetLeader(), 0, 0, MsgStd::LevelSyncDesigneeInOtherArea);
                return;
            }
            else
            {
                for (auto& member : members)
                {
                    if (member->StatusEffectContainer->HasStatusEffect({ EFFECT_LEVEL_RESTRICTION, EFFECT_LEVEL_SYNC, EFFECT_SJ_RESTRICTION, EFFECT_CONFRONTATION, EFFECT_BATTLEFIELD }))
                    {
                        ((CCharEntity*)GetLeader())->pushPacket<CMessageBasicPacket>((CCharEntity*)GetLeader(), (CCharEntity*)GetLeader(), 0, 0, MsgStd::LevelSyncPreventedByStatus);
                        return;
                    }
                }
                m_PSyncTarget = PChar;
                for (auto& i : members)
                {
                    if (i->objtype != TYPE_PC)
                    {
                        continue;
                    }

                    CCharEntity* member = (CCharEntity*)i;

                    if (member->status != STATUS_TYPE::DISAPPEAR && member->getZone() == PChar->getZone())
                    {
                        member->pushPacket<CMessageStandardPacket>(PChar->GetMLevel(), 0, 0, 0, message);
                        member->StatusEffectContainer->AddStatusEffect(new CStatusEffect(EFFECT_LEVEL_SYNC, EFFECT_LEVEL_SYNC, PChar->GetMLevel(), 0s, 0s), EffectNotice::Silent);
                        member->StatusEffectContainer->DelStatusEffectsByFlag(EFFECTFLAG_DISPELABLE | EFFECTFLAG_ON_ZONE);
                        member->loc.zone->PushPacket(member, CHAR_INRANGE, std::make_unique<CCharSyncPacket>(member));
                    }
                }
                db::preparedStmt("UPDATE accounts_parties SET partyflag = partyflag & ~? WHERE partyid = ? AND partyflag & ?",
                                 PARTY_SYNC, m_PartyID, PARTY_SYNC);
                db::preparedStmt("UPDATE accounts_parties SET partyflag = partyflag | ? WHERE partyid = ? AND charid = ?",
                                 PARTY_SYNC, m_PartyID, PChar->id);
                message::send(ipc::PartySetSyncTarget{
                    .partyId = this->GetPartyID(),
                    .charId  = PChar->id,
                });
            }
        }
        else
        {
            if (m_PSyncTarget != nullptr)
            {
                // disable level sync
                for (auto& i : members)
                {
                    if (i->objtype != TYPE_PC)
                    {
                        continue;
                    }

                    CCharEntity* member = (CCharEntity*)i;

                    if (member->status != STATUS_TYPE::DISAPPEAR)
                    {
                        CStatusEffect* sync = member->StatusEffectContainer->GetStatusEffect(EFFECT_LEVEL_SYNC);
                        if (sync && sync->GetDuration() == 0s)
                        {
                            member->pushPacket<CMessageBasicPacket>(member, member, 0, 30, message);
                            sync->SetStartTime(timer::now());
                            sync->SetDuration(30s);
                        }
                    }
                }
            }
            m_PSyncTarget = nullptr;
            message::send(ipc::PartySetSyncTarget{
                .partyId = this->GetPartyID(),
                .charId  = 0,
            });
            db::preparedStmt("UPDATE accounts_parties SET partyflag = partyflag & ~? WHERE partyid = ? AND partyflag & ?",
                             PARTY_SYNC, m_PartyID, PARTY_SYNC);
        }
    }
}

// FIXME: add case for "" membername
void CParty::SetQuarterMaster(const std::string& MemberName)
{
    CBattleEntity* PEntity = GetMemberByName(MemberName);
    m_PQuarterMaster       = PEntity;

    db::preparedStmt("UPDATE accounts_parties SET partyflag = partyflag & ~? WHERE partyid = ? AND partyflag & ?", PARTY_QM, m_PartyID, PARTY_QM);

    if (PEntity != nullptr)
    {
        db::preparedStmt("UPDATE accounts_parties JOIN chars ON accounts_parties.charid = chars.charid "
                         "SET partyflag = partyflag | ? WHERE partyid = ? AND charname = ?",
                         PARTY_QM, m_PartyID, MemberName);
        message::send(ipc::PartySetQuartermaster{
            .partyId = this->GetPartyID(),
            .charId  = PEntity->id,
        });
    }
    else
    {
        message::send(ipc::PartySetQuartermaster{
            .partyId = this->GetPartyID(),
            .charId  = 0,
        });
    }
}

// Send a packet to all members of the group if the zone is specified as 0
// or to the party members in the specified zone.
// Packet for PPartyMember is not sent in both cases
void CParty::PushPacket(uint32 senderID, uint16 ZoneID, const std::unique_ptr<CBasicPacket>& packet)
{
    for (auto& i : members)
    {
        if (i == nullptr || i->objtype != TYPE_PC)
        {
            continue;
        }

        CCharEntity* member = (CCharEntity*)i;

        if (member->id != senderID && member->status != STATUS_TYPE::DISAPPEAR && !jailutils::InPrison(member))
        {
            if (ZoneID == 0 || member->getZone() == ZoneID)
            {
                member->pushPacket(packet->copy());
            }
        }
    }
}

void CParty::DisableSync()
{
    m_PSyncTarget = nullptr;
    // ReloadParty();
}

void CParty::RefreshSync()
{
    CCharEntity* sync      = (CCharEntity*)m_PSyncTarget;
    uint8        syncLevel = sync->jobs.job[sync->GetMJob()];
    if (syncLevel < 10)
    {
        SetSyncTarget("", MsgStd::LevelSyncRemoveLowLevel);
    }
    for (auto& i : members)
    {
        if (i->objtype != TYPE_PC || i->getZone() != sync->getZone())
        {
            continue;
        }

        CCharEntity* member = (CCharEntity*)i;

        uint8 NewMLevel = 0;

        if (syncLevel < member->jobs.job[member->GetMJob()])
        {
            NewMLevel = syncLevel;
        }
        else
        {
            NewMLevel = member->jobs.job[member->GetMJob()];
        }

        if (member->GetMLevel() != NewMLevel)
        {
            charutils::RemoveAllEquipMods(member);
            member->m_LevelRestriction = NewMLevel;
            member->SetMLevel(NewMLevel);
            member->SetSLevel(member->jobs.job[member->GetSJob()]);
            charutils::ApplyAllEquipMods(member);

            blueutils::ValidateBlueSpells(member);
            jobpointutils::RefreshGiftMods(member);
            charutils::BuildingCharSkillsTable(member);
            charutils::CalculateStats(member);
            charutils::BuildingCharTraitsTable(member);
            charutils::BuildingCharAbilityTable(member);
            charutils::BuildingCharWeaponSkills(member);
            charutils::CheckValidEquipment(member);
            member->pushPacket<CCharAbilitiesPacket>(member);
        }
        member->pushPacket<CMessageBasicPacket>(member, member, 0, syncLevel, MsgStd::LevelSyncActivated);
    }
    m_PSyncTarget = sync;
}

void CParty::SetPartyNumber(uint8 number)
{
    m_PartyNumber = number;
}
