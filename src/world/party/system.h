/*
===========================================================================

  Copyright (c) 2025 LandSandBoat Dev Teams

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

#pragma once

#include "party/world.h"

// Retrieve a couple of information about a character from database to make decisions.
class PartySystem
{
public:
    DISALLOW_COPY_AND_MOVE(PartySystem);

    PartySystem(WorldServer& worldServer);
    ~PartySystem() = default;

    WorldParty* getParty(const uint32 partyId);

    void notifyIppForParty(const uint32 partyId, const auto& message, const uint16 zoneId) const;
    void notifyIppForParty(const uint32 partyId, const auto& message) const;

    template <typename Func, typename... Args>
    bool modifyParty(uint32 partyId, Func&& func, Args&&... args);

    void broadcastPartyUpdate(WorldParty& party) const;

    // bool handle_PartyCreate(const IPP& ipp, const ipc::PartyCreate& message)
    // {
    //     auto [it, inserted] = this->m_Parties.emplace(message.charId, WorldParty(message.charId));
    //     ShowInfoFmt("Party created with charId: {}", message.charId);
    //
    //     return handle_PartyAddMember(ipp, ipc::PartyAddMember{ .partyId = message.charId, .charId = message.charId, .type = PartyMemberType::Player });
    // }

    bool handle_CharZoneOut(const IPP& ipp, const ipc::CharZoneOut& message);
    bool handle_CharZoneIn(const IPP& ipp, const ipc::CharZoneIn& message);
    bool handle_PartyAddMember(const IPP& ipp, const ipc::PartyAddMember& message);
    bool handle_PartyRemoveMember(const IPP& ipp, const ipc::PartyRemoveMember& message);
    bool handle_PartyDisband(const IPP& ipp, const ipc::PartyDisband& message);
    bool handle_PartySetLeader(const IPP& ipp, const ipc::PartySetLeader& message);
    bool handle_PartySetQuartermaster(const IPP& ipp, const ipc::PartySetQuartermaster& message);
    bool handle_PartySetSyncTarget(const IPP& ipp, const ipc::PartySetSyncTarget& message);
    bool handle_PartyUpdate(const IPP& ipp, const ipc::PartyUpdate& message);

    void dump();

private:
    WorldServer&                           m_WorldServer;
    std::unordered_map<uint32, WorldParty> m_Parties;

    struct CharDatabaseData
    {
        uint32 charId;
        uint16 zoneId;
        uint8  mJob;
        uint8  mLvl;
        uint8  sJob;
        uint8  sLvl;
    };

    auto getCharInfoFromName(const std::string& name) const -> std::unique_ptr<CharDatabaseData>
    {
        const auto rset = db::preparedStmt("SELECT charid FROM chars WHERE charname = ? LIMIT 1", name);
        FOR_DB_SINGLE_RESULT(rset)
        {
            if (const auto charId = rset->get<uint32>("charid"); charId != 0)
            {
                return getCharInfoFromId(charId);
            }
        }

        ShowErrorFmt("Unable to find target with name: {}", name);
        return nullptr;
    }

    auto getCharInfoFromId(uint32 charId) const -> std::unique_ptr<CharDatabaseData>
    {
        TracyZoneScoped;

        const auto rset = db::preparedStmt(
            "SELECT cs.mjob, cs.mlvl, cs.sjob, cs.slvl, c.pos_zone "
            "FROM char_stats cs "
            "JOIN chars c ON cs.charid = c.charid "
            "WHERE cs.charid = ? "
            "LIMIT 1",
            charId);

        FOR_DB_SINGLE_RESULT(rset)
        {
            auto cdb = std::make_unique<CharDatabaseData>();

            cdb->mJob   = rset->get<uint8>("mjob");
            cdb->mLvl   = rset->get<uint8>("mlvl");
            cdb->sJob   = rset->get<uint8>("sjob");
            cdb->sLvl   = rset->get<uint8>("slvl");
            cdb->zoneId = rset->get<uint16>("pos_zone");
            cdb->charId = charId;

            return cdb;
        }

        return nullptr;
    }
};

template <typename Func, typename... Args>
bool PartySystem::modifyParty(uint32 partyId, Func&& func, Args&&... args)
{
    const auto it = m_Parties.find(partyId);
    if (it == m_Parties.end())
    {
        ShowErrorFmt("Party with ID {} not found", partyId);
        return false;
    }

    const auto result = (it->second.*func)(std::forward<Args>(args)...);

    if (it->second.isDirty())
    {
        broadcastPartyUpdate(it->second);
    }

    return result;
}