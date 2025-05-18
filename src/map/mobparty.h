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

#ifndef _CMOBPARTY_H
#define _CMOBPARTY_H

#include "common/cbasetypes.h"
#include "map_server.h"
#include "packets/message_standard.h"

#include <vector>

class CBasicPacket;
class CMobEntity;
class CCharEntity;
class CAlliance;

/************************************************************************
 *                                                                      *
 *  Character group class                                               *
 *                                                                      *
 ************************************************************************/

class CMobParty
{
public:
    CMobParty(CMobEntity* PEntity);
    CMobParty(uint32 id);
    ~CMobParty();

    uint32 GetPartyID() const;
    uint16 GetMemberFlags(CMobEntity* PEntity);
    uint8  MemberCount(uint16 ZoneID);

    CMobEntity* GetLeader();
    CMobEntity* GetSyncTarget();
    CMobEntity* GetQuaterMaster();
    CMobEntity* GetMemberByName(const std::string& memberName); // Returns entity pointer for member name string

    void DisbandParty(bool playerInitiated = true);
    void ReloadParty();
    void ReloadPartyMembers(CMobEntity* PChar);
    void ReloadTreasurePool(CMobEntity* PChar);

    void   AddMember(CMobEntity* PEntity);
    void   AddMember(uint32 id);                 // Add party member from outside this server's scope
    void   RemoveMember(CMobEntity* PEntity); //
    void   DelMember(CMobEntity* PEntity);    // remove a member without invoking chat/db
    void   PopMember(CMobEntity* PEntity);    // remove a member from memberlist (zoned to different server)
    void   PushMember(CMobEntity* PEntity);   // add a member without invoking chat/db
    void   SetPartyID(uint32 id);                // set new party ID
    void   AssignPartyRole(const std::string& MemberName, uint8 role);
    void   DisableSync();
    void   SetSyncTarget(const std::string& MemberName, MsgStd message);
    void   RefreshSync();
    void   SetPartyNumber(uint8 number);
    bool   HasOnlyOneMember() const;
    bool   IsFull() const;
    uint32 LoadPartySize() const;

    timer::time_point GetTimeLastMemberJoined();
    bool              HasTrusts();

    std::size_t GetMemberCountAcrossAllProcesses();

    void PushPacket(uint32 senderID, uint16 ZoneID, const std::unique_ptr<CBasicPacket>& packet); // Send a packet to all group members, with the exception of PPartyMember
    void PushEffectsPacket();
    void EffectsChanged();

    CAlliance* m_PAlliance;

    // ATTENTION: Do not change the list values outside the party class

    std::vector<CMobEntity*> members;

private:
    struct partyInfo_t;
    uint32    m_PartyID;     // unique party ID
    uint8     m_PartyNumber; // party number in alliance

    CMobEntity* m_PLeader;        // party leader
    CMobEntity* m_PSyncTarget;    // the CMobEntity the party is being synced to
    CMobEntity* m_PQuarterMaster; // the assigned Quartermaster

    bool m_EffectsChanged;

    void                     SetLeader(const std::string& MemberName);        // set party leader
    void                     SetQuarterMaster(const std::string& MemberName); // set Quartermaster
    bool                     RemovePartyLeader(CMobEntity* PEntity);       // attempt to remove the leader of the party. Returns false if party is disbanded or otherwise invalid.
    std::vector<partyInfo_t> GetPartyInfo() const;
    void                     RefreshFlags(std::vector<partyInfo_t>&);
};

#endif
