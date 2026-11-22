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

#include <algorithm>
#include <atomic>
#include <filesystem>
#include <thread>

#include "instance.h"

#include "ai/ai_container.h"
#include "entities/char_entity.h"
#include "lua/luautils.h"
#include "utils/instanceutils.h"
#include "zone.h"

#include "common/timer.h"

CInstance::CInstance(Scheduler& scheduler, MapConfig config, CZone* zone, uint32 instanceid)
: CZoneEntities(scheduler, config, zone)
, m_instanceid(instanceid)
, m_zone(zone)
, m_startTime(timer::now())
, m_lastTimeCheck(m_startTime)
, m_wipeTimer(m_startTime)
{
    TracyZoneScoped;

    static std::atomic<uint32> nextRunId{ 1 };

    runId_ = nextRunId.fetch_add(1, std::memory_order_relaxed);

    LoadInstance();
}

CInstance::~CInstance()
{
    TracyZoneScoped;
}

auto CInstance::GetID() const -> uint16
{
    return m_instanceid;
}

auto CInstance::runId() const -> uint32
{
    return runId_;
}

auto CInstance::GetProgress() const -> uint32
{
    return m_progress;
}

auto CInstance::GetStage() const -> uint32
{
    return m_stage;
}

/************************************************************************
 *                                                                       *
 *  Loads instance settings from the zone's instances.yaml               *
 *                                                                       *
 ************************************************************************/

void CInstance::LoadInstance()
{
    TracyZoneScoped;

    if (!instanceutils::IsValidInstanceID(m_instanceid))
    {
        ShowCritical("CZone::LoadInstance: Cannot load instance %u", m_instanceid);
        Fail();
        return;
    }

    const auto data = instanceutils::GetInstanceData(m_instanceid);

    m_instanceName = data.instance_name;

    m_timeLimit                       = data.time_limit;
    m_entrance                        = data.entrance_zone;
    overlayId_                        = data.overlay;
    m_entryloc                        = data.entry;
    m_zone_music_override.m_songDay   = data.music.Day;
    m_zone_music_override.m_songNight = data.music.Night;
    m_zone_music_override.m_bSongS    = data.music.BattleSolo;
    m_zone_music_override.m_bSongM    = data.music.BattleParty;

    // Add to Lua cache
    // TODO: This will happen more often than needed, but not so often that it's a performance concern
    const auto zone        = m_zone->getName();
    const auto name        = m_instanceName;
    const auto assaultPath = fmt::format("./scripts/assaults/{}/{}.lua", zone, name);
    if (std::filesystem::exists(assaultPath))
    {
        luautils::LoadLuaObjectFromFile(assaultPath, true);
    }
    else
    {
        luautils::LoadLuaObjectFromFile(fmt::format("./scripts/zones/{}/instances/{}.lua", zone, name));
    }
}

/************************************************************************
 *                                                                       *
 *  Registers a char to the char list (and sets first one as leader)     *
 *                                                                       *
 ************************************************************************/

void CInstance::RegisterChar(CCharEntity* PChar)
{
    if (m_registeredChars.empty())
    {
        m_commander = PChar->id;
    }
    m_registeredChars.emplace_back(PChar->id);
}

auto CInstance::GetLevelCap() const -> uint8
{
    return m_levelcap;
}

auto CInstance::GetName() -> const std::string&
{
    return m_instanceName;
}

auto CInstance::GetEntryLoc() -> position_t
{
    return m_entryloc;
}

auto CInstance::GetTimeLimit() -> timer::duration
{
    return m_timeLimit;
}

void CInstance::SetTimeLimit(timer::duration time)
{
    m_timeLimit = time;
}

auto CInstance::GetLastTimeUpdate() -> timer::duration
{
    return m_lastTimeUpdate;
}

auto CInstance::GetWipeTime() -> timer::duration
{
    return m_wipeTimer - m_startTime;
}

auto CInstance::GetElapsedTime(timer::time_point tick) -> timer::duration
{
    // no reason to allow returning a negative elapsed time, can happen if map server is delayed and processing a previous tick
    return std::max(timer::duration(0s), tick - m_startTime);
}

auto CInstance::GetLocalVar(const std::string& name) const -> uint64_t
{
    auto var = localVars_.find(name);
    return var != localVars_.end() ? var->second : 0;
}

void CInstance::SetLevelCap(uint8 cap)
{
    m_levelcap = cap;
}

void CInstance::SetEntryLoc(float x, float y, float z, float rot)
{
    m_entryloc.x        = x;
    m_entryloc.y        = y;
    m_entryloc.z        = z;
    m_entryloc.rotation = (uint8)rot;
}

void CInstance::SetLastTimeUpdate(timer::duration lastTime)
{
    m_lastTimeUpdate = lastTime;
}

void CInstance::SetProgress(uint32 progress)
{
    m_progress = progress;
    luautils::OnInstanceProgressUpdate(this);
}

void CInstance::SetStage(uint32 stage)
{
    m_stage = stage;
}

void CInstance::SetWipeTime(timer::duration time)
{
    m_wipeTimer = time + m_startTime;
}

void CInstance::SetLocalVar(const std::string& name, uint64_t value)
{
    localVars_[name] = value;
}

/************************************************************************
 *                                                                       *
 *  Checks if the instance has expired.  If not, runs instance timer     *
 *                                                                       *
 ************************************************************************/

void CInstance::CheckTime(timer::time_point tick)
{
    auto checkFrequency = 1s;
    // Once someone zones in, m_lastTimeCheck will change and checkFrequency will be pinned at 1s for the remainder of the instance
    if (CharListEmpty() && m_startTime == m_lastTimeCheck)
    {
        // give grace period before first instance check to allow time to register instance and zone in
        // instance.lua lets the client run through the event for a maximum of 35s before forcing zone change with `setPos`
        checkFrequency = 40s;
    }
    if (m_lastTimeCheck + checkFrequency <= tick && !Failed())
    {
        luautils::OnInstanceTimeUpdate(GetZone(), this, static_cast<uint32>(timer::count_seconds(GetElapsedTime(tick))));
        m_lastTimeCheck = tick;
    }
}

auto CInstance::CharRegistered(CCharEntity* PChar) -> bool
{
    return std::ranges::contains(m_registeredChars, PChar->id);
}

void CInstance::ClearEntities()
{
    auto clearStates = [](CBattleEntity* entity)
    {
        if (static_cast<CBattleEntity*>(entity)->isAlive())
        {
            entity->PAI->ClearStateStack();
        }
    };

    ForEachChar(
        [&](CCharEntity* PChar)
        {
            clearStates(PChar);
        });

    ForEachMob(
        [&](CMobEntity* PMob)
        {
            clearStates(PMob);
        });

    ForEachPet(
        [&](CPetEntity* PPet)
        {
            clearStates(PPet);
        });

    ForEachTrust(
        [&](CTrustEntity* PTrust)
        {
            clearStates(PTrust);
        });
}

void CInstance::Fail()
{
    Cancel();

    ClearEntities();

    luautils::OnInstanceFailure(this);
}

auto CInstance::Failed() -> bool
{
    return m_status == INSTANCE_FAILED;
}

void CInstance::Complete()
{
    m_status = INSTANCE_COMPLETE;

    ClearEntities();

    luautils::OnInstanceComplete(this);
}

auto CInstance::Completed() -> bool
{
    return m_status == INSTANCE_COMPLETE;
}

void CInstance::Cancel()
{
    m_status = INSTANCE_FAILED;
}

auto CInstance::CheckFirstEntry(uint32 id) -> bool
{
    // insert returns a pair (iterator,inserted)
    return m_enteredChars.insert(id).second;
}

auto CInstance::GetSoloBattleMusic() -> uint16
{
    return m_zone_music_override.m_bSongS ? *m_zone_music_override.m_bSongS : GetZone()->GetSoloBattleMusic();
}

auto CInstance::GetPartyBattleMusic() -> uint16
{
    return m_zone_music_override.m_bSongM ? *m_zone_music_override.m_bSongM : GetZone()->GetPartyBattleMusic();
}

auto CInstance::GetBackgroundMusicDay() -> uint16
{
    return m_zone_music_override.m_songDay ? *m_zone_music_override.m_songDay : GetZone()->GetBackgroundMusicDay();
}

auto CInstance::GetBackgroundMusicNight() -> uint16
{
    return m_zone_music_override.m_songNight ? *m_zone_music_override.m_songNight : GetZone()->GetBackgroundMusicNight();
}

// Certain instances with multiple sub-maps use alternative entity lists replacing the base list
// The client needs it to know which DATs to load.
auto CInstance::overlayId() const -> uint32
{
    return overlayId_;
}
