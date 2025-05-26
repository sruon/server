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
    if (getSyncTarget() && getSyncTarget()->get().getId() == charId)
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

bool WorldParty::setLeader(uint32_t UniqueNo)
{
    for (auto& member : getMembers())
    {
        if (member.get().getId() == UniqueNo)
        {
            m_LeaderUniqueNo = UniqueNo;
            m_PartyId        = UniqueNo;
            ShowInfoFmt("Leader set to UniqueNo: {}, changed PartyId", UniqueNo);
            setDirty(true);
            return true;
        }
    }

    ShowWarningFmt("Member with UniqueNo: {} not found in party", UniqueNo);
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

bool WorldParty::setSyncTarget(uint32_t UniqueNo)
{
    if (UniqueNo == 0)
    {
        m_SyncTargetUniqueNo = 0;
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

bool WorldParty::addMember(uint32_t UniqueNo, PartyMemberType type, const uint32 ZoneId)
{
    for (const auto& member : getMembers())
    {
        if (member.get().getId() == UniqueNo)
        {
            ShowWarningFmt("Member with UniqueNo: {} already exists in the party", UniqueNo);
            return false;
        }
    }

    if (!isFull())
    {
        // Capture PC names. Not relevant for trusts.
        std::string charName = "";
        if (type == PartyMemberType::Player)
        {
            const auto rset = db::preparedStmt("SELECT charname FROM chars WHERE charid = ?", UniqueNo);
            if (rset && rset->rowsCount() && rset->next())
            {
                charName = rset->get<std::string>("charname");
            }
        }

        m_Members.emplace_back(UniqueNo, type, ZoneId, charName, std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
        ShowInfoFmt("Added member {} ({}) (type {})", charName, UniqueNo, static_cast<uint8>(type));

        // If we added a player (that's not the initial member), all trusts should be cleared.
        if (type == PartyMemberType::Player && getMemberCount() > 1)
        {
            clearTrusts();
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
        reassignLeader();
    }

    if (m_QuartermasterUniqueNo == UniqueNo)
    {
        setQuartermaster(0);
    }

    if (m_SyncTargetUniqueNo == UniqueNo)
    {
        setSyncTarget(0);
    }

    ShowInfoFmt("Removed member with UniqueNo: {}", UniqueNo);
    setDirty(true);
    return true;
}