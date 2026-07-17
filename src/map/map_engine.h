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

#pragma once

#include "pch.h"

#include <common/application.h>
#include <common/ipp.h>
#include <common/timer.h>

#include "map_config.h"
#include "zone.h"

//
// Forward Declarations
//

class IPP;
class IPCClient;
class MapNetworking;
class MapStatistics;
class CZone;

//
// Exposed globals
//

extern std::map<uint16, CZone*> g_PZoneList; // Global array of pointers for zones

class MapEngine final : public Engine
{
public:
    MapEngine(Application& application, MapConfig& config);
    ~MapEngine() override;

    //
    // Tasks
    //

    auto init() -> Task<void>;
    auto watchdogUpdater() -> Task<void>;
    auto watchdogWatcher() -> Task<void>;

    //
    // Maintenance
    //

    // TODO: Turn these into real coroutines
    void sessionCleanup() const;
    void garbageCollect() const;

    //
    // Commands callbacks
    //

    void onStats(std::vector<std::string>& inputs) const;
    void onBacktrace(std::vector<std::string>& inputs) const;
    void onReloadRecipes(std::vector<std::string>& inputs) const;
    void onGM(const std::vector<std::string>& inputs) const;

    //
    // Accessors
    //

    auto networking() const -> MapNetworking&;
    auto statistics() const -> MapStatistics&;
    auto scheduler() -> Scheduler&;
    auto zones() const -> std::map<uint16, CZone*>&; // g_PZoneList
    auto config() const -> MapConfig&;
    // TODO: gameState()

private:
    Application& application_;
    Scheduler&   scheduler_;

    Maybe<Scheduler::Token> mapCleanupToken_;
    Maybe<Scheduler::Token> mapGarbageCollectToken_;
    Maybe<Scheduler::Token> timeServerToken_;
    Maybe<Scheduler::Token> persistVolatileServerVarsToken_;
    Maybe<Scheduler::Token> pumpIPCToken_;
    Maybe<Scheduler::Token> flushStatisticsToken_;
    Maybe<Scheduler::Token> livingWorldHeartbeatToken_; // --keep-zones-awake: periodic zone-liveness log
    Maybe<Scheduler::Token> tickBudgetProbeToken_;      // --keep-zones-awake: main-loop tick-slip detector

    // Tick-budget probe state (main-loop slip detection for the living-world viability
    // question). A main-thread task on the zone-tick cadence (kLogicUpdateInterval)
    // resumes late when the single main thread is saturated; the gap beyond budget is
    // the slip. Windowed stats reset by each heartbeat.
    std::chrono::steady_clock::time_point lastTickProbe_{};
    uint32 tickProbeCount_{ 0 };
    uint32 tickOverrunCount_{ 0 }; // gap > 125% of budget (a slipped tick)
    uint32 tickSevereCount_{ 0 };  // gap > 200% of budget (a badly slipped tick)
    int64  tickOverrunMaxMs_{ 0 }; // worst slip (gap - budget)
    int64  tickGapMaxMs_{ 0 };     // worst raw gap seen this window
    int64  tickGapSumMs_{ 0 };

    std::unique_ptr<MapStatistics> mapStatistics_;
    std::unique_ptr<MapNetworking> networking_;
    std::unique_ptr<IPCClient>     ipcClient_;
    std::atomic<timer::time_point> watchdogLastUpdate_;
    MapConfig&                     config_;
};
