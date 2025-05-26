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

#include "common/cbasetypes.h"
#include "common/ipc.h"
#include "common/party/base.h"

class CCharEntity;
class CBattleEntity;
enum class PartyFlag : uint16;

// This is a read-only view of a party of CCharEntity
// Updates are only permitted through the IPC interface
// The nested IpcHelper class is used to send messages to the world server
// Keep in mind that several map processes _may_ be performing similar operations.
// Therefore, operations should be strictly limited to players on this process.
// Nevertheless, the underlying data has knowledge of all members across all processes and can be used to make decisions.
class CCharParty : public PartyBase
{
public:
    DISALLOW_COPY_AND_MOVE(CCharParty);

    // All map->world communications go through the IpcHelper
    class IpcHelper;
    auto ipc() const -> const IpcHelper&;

    static std::unique_ptr<CCharParty> Create(const ipc::PartyUpdate& message)
    {
        return std::unique_ptr<CCharParty>(new CCharParty(message));
    }

    ~CCharParty();
    void refreshSync(CCharEntity* PChar) const;
    void refreshSync() const;

    // Helpers
    bool hasJob(uint8 job, std::optional<uint16> zoneId = std::nullopt) const;

    // Packets
    void broadcastPartyPackets(const CCharEntity* PSingle = nullptr);
    void chatMessage(const ipc::ChatMessageParty& message) const;
    void chatMessage(const ipc::ChatMessageAlliance& message) const;

    void pushPacket(uint32 senderID, uint16 ZoneID, const std::unique_ptr<CBasicPacket>& packet) const;
    void pushEffectsPacket(CCharEntity* PChar) const;

    // Members retrieval
    auto getMembers(PartyMemberFilter filter = {}) const -> std::vector<CBattleEntity*>;
    auto getPlayers(PartyMemberFilter filter = {}) const -> std::vector<CCharEntity*>;
    auto getLeader() const -> CCharEntity*;
    auto getSyncTarget() const -> CCharEntity*;
    auto getQuartermaster() const -> CCharEntity*;
    auto getMemberByName(const std::string& memberName) const -> CCharEntity*;
    auto getMemberById(uint32 charId) const -> CCharEntity*;

    // Iterators
    void ForEveryMember(const std::function<void(CCharEntity*)>& func) const;
    void ForEveryMemberWithTrusts(const std::function<void(CBattleEntity*)>& func) const;
    void ForEveryAllianceMember(std::function<void(CCharEntity*)> func);

private:
    CCharParty(const ipc::PartyUpdate& message);

    void setPartyId(uint32 partyId);

    void applySync(CCharEntity* PChar) const;
    void disableSync(const CCharEntity* PChar) const;

    void update(const ipc::PartyUpdate& message);
    void addMember(const PartyMember& member);
    void delMember(const PartyMember& member);

    std::unique_ptr<IpcHelper> m_pIpcHelper;

    // Allow only PartyContainer to call update() to enforce the read-only nature
    friend class PartyContainer;
};

#include "ipc_helper.h"