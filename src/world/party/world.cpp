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

#include "world.h"

#include "common/database.h"
#include "common/ipc_structs.h"
#include "common/logging.h"

struct CharDatabaseData
{
    std::string charName{};
    uint32      charId{};
    uint16      zoneId{};
    uint8       mJob{};
    uint8       mLvl{};
    uint8       sJob{};
    uint8       sLvl{};
};

auto getCharInfoFromId(uint32 charId) -> std::unique_ptr<CharDatabaseData>
{
    TracyZoneScoped;

    const auto rset = db::preparedStmt(
        "SELECT cs.mjob, cs.mlvl, cs.sjob, cs.slvl, c.pos_zone, c.charname "
        "FROM char_stats cs "
        "JOIN chars c ON cs.charid = c.charid "
        "WHERE cs.charid = ? "
        "LIMIT 1",
        charId);

    FOR_DB_SINGLE_RESULT(rset)
    {
        auto cdb = std::make_unique<CharDatabaseData>();

        cdb->mJob     = rset->get<uint8>("mjob");
        cdb->mLvl     = rset->get<uint8>("mlvl");
        cdb->sJob     = rset->get<uint8>("sjob");
        cdb->sLvl     = rset->get<uint8>("slvl");
        cdb->zoneId   = rset->get<uint16>("pos_zone");
        cdb->charId   = charId;
        cdb->charName = rset->get<std::string>("charname");

        return cdb;
    }

    return nullptr;
}

auto getCharInfoFromName(const std::string& name) -> std::unique_ptr<CharDatabaseData>
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

WorldParty::WorldParty(const ipc::PartyUpdate& message)
: PartyBase(message)
{
}

WorldParty::WorldParty(uint32 _LeaderUniqueNo)
: PartyBase(_LeaderUniqueNo)
{
}

bool WorldParty::setMemberZone(const uint32 charId, const uint16 zoneId)
{
    // if (const auto syncTarget = party.getSyncTarget())
    //     {
    //         if (const PartyMember& target = syncTarget.value(); target.getId() == message.charId)
    //         {
    //             ShowInfoFmt("Sync target is zoning out. Removing sync target");
    //             handle_PartySetSyncTarget(ipp, ipc::PartySetSyncTarget{ .partyId = party.getPartyId(), .charId = 0, .reason = MsgStd::LevelSyncDeactivateLeftArea });
    //         }
    //         else
    //         {
    //             // Someone is zoning out, check if enough members are left in the sync zone.
    //             if (party.getMembers({ .zoneId = target.getZone() }).size() < 2)
    //             {
    //                 // Not enough players left, disable sync
    //                 handle_PartySetSyncTarget(ipp, ipc::PartySetSyncTarget{ .partyId = party.getPartyId(), .charId = 0, .reason = MsgStd::LevelSyncRemoveTooFewMembers });
    //             }
    //         }
    //     }
    //
    //     // Leader is zoning out, clear all trusts
    //     if (party.getLeaderId() == message.charId)
    //     {
    //         party.clearTrusts();
    //     }
    //
    //     if (message.destinationZoneId == 0xFFFF)
    //     {
    //         // Character is shutting down / logging out
    //         return handle_PartyRemoveMember(ipp, ipc::PartyRemoveMember{ .partyId = party.getPartyId(), .charId = message.charId });
    //     }
    //
    //     // May need to reenable if we get odd behavior in the timeframe between a char zoning out and zoning in.
    //     return modifyParty(party.getPartyId(), &WorldParty::setMemberZone, message.charId, message.destinationZoneId);
    //     // return true;
    // }

    if (getLeaderId() == charId)
    {
        clearTrusts();
    }

    if (getSyncTargetId() == charId)
    {
        setSyncTarget(0);
    }

    if (zoneId == 0xFFFF) // Logging out
    {
        return removeMember(charId);
    }

    for (auto& member : getMembers())
    {
        if (member.get().getId() == charId)
        {
            member.get().setZone(zoneId);
            setDirty(true);
            return true;
        }
    }

    return false;
}

bool WorldParty::setLeader(const std::string& charName)
{
    if (const auto existingMember = getMemberByName(charName))
    {
        return setLeader(existingMember->get().getId());
    }

    ShowErrorFmt("Unable to find target member with name: {}", charName);
    return false;
}

bool WorldParty::setLeader(uint32_t UniqueNo)
{
    // TODO: Party container key
    const auto maybeMember = getMemberById(UniqueNo);

    if (maybeMember && maybeMember->get().getType() == PartyMemberType::Player)
    {
        m_LeaderUniqueNo = UniqueNo;
        m_PartyId        = UniqueNo;
        ShowInfoFmt("Leader set to UniqueNo: {}, changed PartyId", UniqueNo);
        // Changing leader dismisses trusts
        clearTrusts();
        setDirty(true);
        return true;
    }

    ShowWarningFmt("Member with UniqueNo: {} not found in party", UniqueNo);
    return false;
}

bool WorldParty::setQuartermaster(const std::string& charName)
{
    if (const auto existingMember = getMemberByName(charName))
    {
        return setQuartermaster(existingMember->get().getId());
    }

    ShowErrorFmt("Unable to find target member with name: {}", charName);
    return false;
}

bool WorldParty::setQuartermaster(uint32_t UniqueNo)
{
    if (UniqueNo == 0)
    {
        m_QuartermasterUniqueNo = 0;
        ShowInfo("Quartermaster removed");
        setDirty(true);
        return true;
    }

    for (auto& member : getMembers())
    {
        if (member.get().getId() == UniqueNo)
        {
            m_QuartermasterUniqueNo = UniqueNo;
            ShowInfoFmt("Quartermaster set to UniqueNo: {}", UniqueNo);
            setDirty(true);
            return true;
        }
    }

    ShowWarningFmt("Member with UniqueNo: {} not found in party", UniqueNo);
    return false;
}

bool WorldParty::setSyncTarget(const std::string& charName)
{
    if (const auto existingMember = getMemberByName(charName))
    {
        return setSyncTarget(existingMember->get().getId());
    }

    ShowErrorFmt("Unable to find target member with name: {}", charName);
    return false;
}

bool WorldParty::setSyncTarget(uint32_t UniqueNo)
{
    if (UniqueNo == 0)
    {
        m_SyncTargetUniqueNo = 0;
        //  // Level sync rules enforcement
        // if (const auto maybeLeader = party.getLeader())
        // {
        //     const PartyMember& leader = maybeLeader.value();

        // 1. Sync target must be in the same zone as the party leader
        // if (syncTargetInfo->zoneId != leader.getZone())
        // {
        //     m_WorldServer.ipcServer_->rerouteMessageToCharId(
        //         leader.getId(),
        //         ipc::MessageBasic{
        //             .recipientId = leader.getId(),
        //             .message     = MsgStd::LevelSyncDesigneeInOtherArea,
        //         });
        //
        //     return false;
        // }

        // 2. Sync target must be above level 10
        // if (syncTargetInfo->mLvl < 10)
        // {
        //     m_WorldServer.ipcServer_->rerouteMessageToCharId(
        //         leader.getId(),
        //         ipc::MessageBasic{
        //             .recipientId = leader.getId(),
        //             .message     = MsgStd::LevelSyncDesigneeBelowMin,
        //             .param1      = 10,
        //         });
        //
        //     return false;
        // }

        // 3. Certain status effects block sync
        // TODO: Map server is authoritative for this check in SmallPacket0x077
        // Status effects are not saved to database unless specific actions are performed, so we are unable to get a proper view from this side.

        // 4. If the target is alone in the zone, the sync is IMMEDIATELY disabled.
        // Verified to be retail accurate.
        // if (party.getMembers({ .zoneId = syncTargetInfo->zoneId }).size() < 2)
        // {
        //     immediateDisable = true;
        // }
        // // If a reason was provided for disabling, we need to stream it to certain party members
        //    if (maybeOldSync && syncTargetId == 0 && static_cast<uint16>(message.reason) != 0)
        //    {
        //        const PartyMember& oldSync = maybeOldSync.value();
        //
        //        // clang-format off
        //        party.ForEveryMember({ .zoneId = oldSync.getZone() }, [&](const PartyMember& member)
        //        {
        //            m_WorldServer.ipcServer_->rerouteMessageToCharId(member.getId(),
        //                ipc::MessageBasic{
        //                    .recipientId = member.getId(),
        //                    .message     = message.reason,
        //                    .param0      = 30,
        //                    .param1      = 30,
        //                });
        //        });
        //        // clang-format on
        //    }
        ShowInfo("Sync target removed");
        setDirty(true);
        return true;
    }

    for (auto& member : getMembers())
    {
        if (member.get().getId() == UniqueNo)
        {
            m_SyncTargetUniqueNo = UniqueNo;
            ShowInfoFmt("Sync target set to UniqueNo: {}", UniqueNo);
            setDirty(true);
            return true;
        }
    }

    ShowWarningFmt("Member with UniqueNo: {} not found in party", UniqueNo);
    return false;
}

bool WorldParty::addMember(uint32_t UniqueNo, PartyMemberType type)
{
    if (getMemberById(UniqueNo))
    {
        ShowWarningFmt("Member with UniqueNo: {} already exists in the party", UniqueNo);
        return false;
    }

    if (!isFull())
    {
        if (type == PartyMemberType::Player)
        {
            const auto newMemberInfo = getCharInfoFromId(UniqueNo);
            if (!newMemberInfo)
            {
                ShowErrorFmt("Unable to find target member with ID: {}", UniqueNo);
                return false;
            }
            m_Members.emplace_back(UniqueNo, type, newMemberInfo->zoneId, newMemberInfo->charName, std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
            ShowInfoFmt("Added player {} ({}) (type {})", newMemberInfo->charName, UniqueNo, static_cast<uint8>(type));

            // Adding a player dismisses all trusts, regardless of the state of the party.
            clearTrusts();
        }
        else
        {
            // For trusts, we don't care about their name.
            // (At least while the map server is authoritative on MOST trust checks.)
            // Jury is still out on whether we want the whole trust checks to be handled on world server.
            // Their zoneId is still important, though.
            // The trusts are ALWAYS attached to the leader.
            // TODO: Rework this ugly call + optional check for edge cases.
            m_Members.emplace_back(UniqueNo, type, getLeader().value().get().getZone(), "TRUST", std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
        }

        setDirty(true);
        return true;
    }

    ShowWarningFmt("Party is full, cannot add member with UniqueNo: {}", UniqueNo);
    return false;
}

void WorldParty::clearTrusts()
{
    // Trusts can't be QM/Leader/SyncTarget, so we can safely remove them without checking.

    // clang-format off
    const auto removed = std::erase_if(m_Members, [](const auto& member)
    {
        return member.getType() == PartyMemberType::Trust;
    });
    // clang-format on

    if (removed > 0)
    {
        setDirty(true);
    }
}

bool WorldParty::removeMember(const std::string& charName)
{
    if (const auto existingMember = getMemberByName(charName))
    {
        return removeMember(existingMember->get().getId());
    }

    ShowErrorFmt("Unable to find target member with name: {}", charName);
    return false;
}

bool WorldParty::removeMember(uint32 UniqueNo)
{
    // clang-format off
    const auto it = std::ranges::find_if(m_Members,
         [UniqueNo](const PartyMember& member)
         {
             return member.getId() == UniqueNo;
         });
    // clang-format on
    if (it == m_Members.end())
    {
        ShowWarningFmt("Member with UniqueNo: {} not found in party", UniqueNo);
        return false;
    }

    m_Members.erase(it);

    // Handle special role reassignments
    if (m_LeaderUniqueNo == UniqueNo)
    {
        // TODO: Party system key
        reassignLeader();
    }

    if (m_QuartermasterUniqueNo == UniqueNo)
    {
        setQuartermaster(0);
    }

    if (m_SyncTargetUniqueNo == UniqueNo)
    {
        // TODO: MsgStd::LevelSyncRemoveLeftParty
        setSyncTarget(0);
    }

    ShowInfoFmt("Removed member with UniqueNo: {}", UniqueNo);
    setDirty(true);
    return true;
}

bool WorldParty::disband()
{
    for (auto& member : getMembers())
    {
        removeMember(member.get().getId());
    }

    return true;
}