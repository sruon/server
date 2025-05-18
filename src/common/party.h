//
// Created by sruon on 5/15/2025.
//

#pragma once

#include "common/cbasetypes.h"
#include "timer.h"

enum class PartyMemberType : uint8
{
    Player,
    Trust,
};

struct SerializablePartyMember
{
    uint32          UniqueNo;
    std::time_t     JoinedTime;
    PartyMemberType Type;
};

struct PartyMemberData
{
    uint32          UniqueNo;
    std::time_t     JoinedTime;
    PartyMemberType Type;
    uint32          ZoneId;
};

class PartyMember
{
public:
    PartyMember(const uint32 _UniqueNo, const PartyMemberType _type, const uint32 _ZoneId)
    {
        m_Data.UniqueNo   = _UniqueNo;
        m_Data.JoinedTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        m_Data.Type       = _type;
        m_Data.ZoneId     = _ZoneId;
    }

    PartyMember(PartyMemberData& data)
    {
        m_Data = data;
    }

    auto GetType() const -> PartyMemberType
    {
        return m_Data.Type;
    }

    auto GetId() const -> uint32
    {
        return m_Data.UniqueNo;
    }

    auto GetZone() const -> uint32
    {
        return m_Data.ZoneId;
    }

    auto GetTimeSinceJoined() const -> std::chrono::seconds
    {
        // TODO: Should be steady_clock but alpaca only deals with time_t hmmmmmm
        const auto joinedTimePoint = std::chrono::system_clock::from_time_t(m_Data.JoinedTime);
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - joinedTimePoint);
    }

    PartyMemberData Serializable() const
    {
        return m_Data;
    }

private:
    // Stuffing everything in a struct that can be serialized since alpaca is super finnicky
    PartyMemberData m_Data{};
};
