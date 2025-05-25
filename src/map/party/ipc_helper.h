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
#include "party/char_party.h"

enum class PartyMemberType : uint8;
class CBattleEntity;
class CCharEntity;

class CCharParty::IpcHelper
{
public:
    static std::unique_ptr<IpcHelper> Create(const CCharParty& party)
    {
        return std::unique_ptr<IpcHelper>(new IpcHelper(party));
    }

    void setLeader(uint32 UniqueNo) const;
    void setLeader(const std::string& charName) const;

    void setSyncTarget(const std::string& CharName) const;
    void setSyncTarget(uint32 UniqueNo) const;
    void clearSyncTarget(MsgStd Reason) const;

    void setQuartermaster(uint32 UniqueNo) const;
    void setQuartermaster(const std::string& charName) const;

    void addMember(uint32 UniqueNo, PartyMemberType Type) const;

    void removeMember(uint32 UniqueNo) const;

    void disband() const;

    void notifyKick(uint32 UniqueNo) const;

protected:
    IpcHelper(const CCharParty& party)
    : m_Party(party)
    {
    }

private:
    const CCharParty& m_Party;
    friend class CCharParty;
};