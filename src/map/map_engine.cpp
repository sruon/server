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

#include "map_engine.h"

#include "common/debug.h"
#include "common/ipp.h"
#include "common/logging.h"
#include "common/settings.h"
#include "common/timer.h"
#include "common/utils.h"
#include "common/vana_time.h"
#include "common/version.h"
#include "common/zlib.h"

#include "ability.h"
#include "daily_system.h"
#include "ipc_client.h"
#include "job_points.h"
#include "map_networking.h"
#include "map_statistics.h"
#include "mob_spell_list.h"
#include "monstrosity.h"
#include "roe.h"
#include "spell.h"
#include "status_effect_container.h"
#include "time_server.h"
#include "transport.h"
#include "zone.h"
#include "zone_entities.h"

#include "ai/controllers/automaton_controller.h"

#include "items/item_equipment.h"

#include "packets/s2c/0x017_chat_std.h"

#include "utils/battleutils.h"
#include "utils/charutils.h"
#include "utils/fishingutils.h"
#include "utils/gardenutils.h"
#include "utils/guildutils.h"
#include "utils/instanceutils.h"
#include "utils/itemutils.h"
#include "utils/mobutils.h"
#include "utils/moduleutils.h"
#include "utils/petutils.h"
#include "utils/serverutils.h"
#include "utils/synergyutils.h"
#include "utils/synthutils.h"
#include "utils/trustutils.h"
#include "utils/zoneutils.h"
#include "utils/lw_profile.h"
#include "zone_entities.h"
#include "entities/base_entity.h"

#include <algorithm>
#include <cstdlib>
#include <string>
#include <vector>
#include <thread>

#ifdef _WIN32
#include <io.h>
#endif

MapEngine::MapEngine(Application& application, MapConfig& config)
: application_(application)
, scheduler_(application_.scheduler())
, mapStatistics_(std::make_unique<MapStatistics>())
, networking_(std::make_unique<MapNetworking>(scheduler_, *mapStatistics_, config))
, config_(config)
{
}

MapEngine::~MapEngine()
{
    itemutils::FreeItemList();
    battleutils::FreeWeaponSkillsList();
    battleutils::FreeMobSkillList();
    battleutils::FreePetSkillList();
    fishingutils::CleanupFishing();
    guildutils::Cleanup();
    mobutils::Cleanup();
    traits::ClearTraitsList();

    petutils::FreePetList();
    zoneutils::FreeZoneList();

    luautils::cleanup();
}

auto MapEngine::init() -> Task<void>
{
    TracyZoneScoped;

#ifdef TRACY_ENABLE
    ShowInfo("*** TRACY IS ENABLED ***");
#endif // TRACY_ENABLE

    ShowInfo(fmt::format("Last Branch: {}", version::GetGitBranch()));
    ShowInfo(fmt::format("SHA: {} ({})", version::GetGitSha(), version::GetGitDate()));

    ShowInfo("do_init: begin server initialization");

    const auto mapIPP = networking_->ipp();

    ShowInfoFmt("map_ip: {}", mapIPP.getIPString());
    ShowInfoFmt("map_port: {}", mapIPP.getPort());

    ShowInfoFmt("Zones assigned to this process: {}", zoneutils::GetZonesAssignedToThisProcess(mapIPP).size());

    ShowInfo(fmt::format("Random samples (integer): {}", utils::getRandomSampleString(0, 255)));
    ShowInfo(fmt::format("Random samples (float): {}", utils::getRandomSampleString(0.0f, 1.0f)));

    ShowInfo("do_init: connecting to database");
    ShowInfo(fmt::format("database name: {}", db::getDatabaseSchema()).c_str());
    ShowInfo(fmt::format("database server version: {}", db::getDatabaseVersion()).c_str());
    ShowInfo(fmt::format("database client version: {}", db::getDriverVersion()).c_str());

    if (!config_.inCI)
    {
        db::checkCharset();
    }

    db::checkTriggers();

    luautils::init(mapIPP, config_.inCI); // Also calls moduleutils::LoadLuaModules();

    // Delete sessions that are associated with this map process, but leave others alone
    db::preparedStmt("DELETE FROM accounts_sessions WHERE IF(? = 0 AND ? = 0, true, server_addr = ? AND server_port = ?)",
                     mapIPP.getIP(),
                     mapIPP.getPort(),
                     mapIPP.getIP(),
                     mapIPP.getPort());

    ShowInfo("do_init: zlib is reading");
    zlib_init();

    ShowInfo("do_init: starting ZMQ thread");
    ipcClient_ = std::make_unique<IPCClient>(networking(), application_.zmqService());
    message::init(*ipcClient_);

    // NOTE: We're phasing out server usage without ximeshes and navmeshes. For now for ease of use,
    // we're still allowing it in CI, but regular usage will demand them.
    if (!config_.inCI)
    {
        if (!std::filesystem::exists("./ximeshes/") || std::filesystem::is_empty("./ximeshes/"))
        {
            ShowCritical("./ximeshes/ directory isn't present or is empty! Check your setup.");
            std::exit(-1);
        }

        if (!std::filesystem::exists("./navmeshes/") || std::filesystem::is_empty("./navmeshes/"))
        {
            ShowCritical("./navmeshes/ directory isn't present or is empty! Check your setup.");
            std::exit(-1);
        }
    }

    ShowInfo("do_init: loading items");
    itemutils::Initialize();

    ShowInfo("do_init: loading plants");
    gardenutils::Initialize();

    ShowInfo("do_init: loading spells");
    spell::LoadSpellList();
    mobSpellList::LoadMobSpellList();
    automaton::LoadAutomatonSpellList();
    automaton::LoadAutomatonAbilities();

    guildutils::Initialize();
    charutils::LoadExpTable();
    traits::LoadTraitsList();
    effects::LoadEffectsParameters();
    battleutils::LoadSkillTable();
    meritNameSpace::LoadMeritsList();
    ability::LoadAbilitiesList();
    battleutils::LoadWeaponSkillsList();
    battleutils::LoadMobSkillsList();
    battleutils::LoadPetSkillsList();
    petutils::LoadPetList();
    trustutils::LoadTrustList();
    mobutils::LoadSqlModifiers();
    jobpointutils::LoadGifts();
    daily::LoadDailyItems();
    roeutils::UpdateUnityRankings();
    synthutils::LoadSynthRecipes();
    synergyutils::LoadSynergyRecipes();
    CItemEquipment::LoadAugmentData(); // TODO: Move to itemutils

    co_await zoneutils::Initialize(scheduler_, config_);
    instanceutils::Initialize(config_);

    if (!config_.lazyZones)
    {
        CTransportHandler::getInstance()->InitializeTransport(mapIPP);
    }

    fishingutils::InitializeFishingSystem();

    monstrosity::LoadStaticData();

    if (!config_.controlledWeather)
    {
        zoneutils::InitializeWeather(); // Need VanaTime initialized
    }

    //
    // Queue up regular tasks for the Scheduler
    //

    if (!config_.isTestServer)
    {
        mapCleanupToken_        = scheduler_.intervalOnMainThread(kSessionCleanupInterval, std::bind(&MapEngine::sessionCleanup, this));
        mapGarbageCollectToken_ = scheduler_.intervalOnMainThread(kGarbageCollectionInterval, std::bind(&MapEngine::garbageCollect, this));

        timeServerToken_ = scheduler_.intervalOnMainThread(
            kTimeServerTickInterval,
            [this]() -> Task<void>
            {
                co_await time_server(scheduler_, config_);
            });

        persistVolatileServerVarsToken_ = scheduler_.intervalOnMainThread(kPersistVolatileServerVarsInterval, serverutils::PersistVolatileServerVars);
        pumpIPCToken_                   = scheduler_.intervalOnMainThread(kIPCPumpInterval, message::handle_incoming);
        flushStatisticsToken_           = scheduler_.intervalOnMainThread(kTimeServerTickInterval, std::bind(&MapNetworking::flushStatistics, networking_.get()));
    }

    // Living-world viability instrumentation (--keep-zones-awake). The question this
    // answers: can a single main thread keep all 300 zones ticking on the 400ms logic
    // cadence with ~100 players, or does the tick budget slip? Two main-thread tasks:
    //   1. A tick-budget probe on the logic cadence. Because interval tasks are
    //      relative (run, then yieldFor(interval)), this probe resumes late exactly
    //      when the main thread is saturated; gap-beyond-budget is the slip.
    //   2. A 30s heartbeat that logs zone liveness + population and the windowed slip
    //      stats, then resets the window.
    if (config_.keepZonesAwake && !config_.isTestServer)
    {
        const auto budgetMs = std::chrono::duration_cast<std::chrono::milliseconds>(kLogicUpdateInterval).count();

        lastTickProbe_        = std::chrono::steady_clock::now();
        tickBudgetProbeToken_ = scheduler_.intervalOnMainThread(
            kLogicUpdateInterval,
            [this, budgetMs]() -> void
            {
                const auto now   = std::chrono::steady_clock::now();
                const auto gapMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTickProbe_).count();
                lastTickProbe_   = now;

                const auto overMs = gapMs - budgetMs;
                ++tickProbeCount_;
                tickGapSumMs_  = tickGapSumMs_ + gapMs;
                tickGapMaxMs_  = std::max<int64>(tickGapMaxMs_, gapMs);
                if (overMs > budgetMs / 4) // >25% beyond the 400ms budget = a slipped tick
                {
                    ++tickOverrunCount_;
                    tickOverrunMaxMs_ = std::max<int64>(tickOverrunMaxMs_, overMs);
                    if (overMs > budgetMs) // >=2x budget: a bad slip — count + surface it
                    {
                        ++tickSevereCount_;
                        ShowWarningFmt("living-world tick slip: {}ms gap ({}ms over the {}ms budget)", gapMs, overMs, budgetMs);
                    }
                }
            });

        // Emit on the same cadence as the metrics scrape (10s) so Grafana sees fresh
        // per-window tick stats. Human line + a machine-readable `lwmetrics` key=val
        // line the Prometheus exporter parses.
        livingWorldHeartbeatToken_ = scheduler_.intervalOnMainThread(
            std::chrono::seconds(10),
            [this, budgetMs]() -> void
            {
                uint32 total = 0, ticking = 0, players = 0, mobs = 0, npcs = 0, inCombat = 0;
                // Per-zone profiling: cadence adherence (interval between tick starts
                // vs the 400ms budget — the honest "is this zone on schedule" signal)
                // and real AI CPU (synchronous LOS/roam/path attributed by zone).
                struct ZoneCost { uint16 id; int64 interval_us; int64 ai_us; uint32 mobs; uint32 players; std::string name; };
                std::vector<ZoneCost> costs;
                zoneutils::ForEachZone(
                    [&](CZone* PZone)
                    {
                        if (PZone == nullptr || PZone->GetIP() == 0)
                        {
                            return;
                        }
                        ++total;
                        if (PZone->IsTicking())
                        {
                            ++ticking;
                        }
                        uint32 zplayers = 0, zmobs = 0;
                        if (auto* ents = PZone->GetZoneEntities())
                        {
                            zplayers = static_cast<uint32>(ents->GetCharList().size());
                            zmobs    = static_cast<uint32>(ents->GetMobList().size());
                            players += zplayers;
                            mobs += zmobs;
                            npcs += static_cast<uint32>(ents->GetNpcList().size());
                            for (const auto& [tid, ent] : ents->GetCharList())
                            {
                                if (ent != nullptr && ent->animation == ANIMATION_ATTACK)
                                {
                                    ++inCombat;
                                }
                            }
                        }
                        const auto zid = static_cast<uint16>(PZone->GetID());
                        costs.push_back({ zid, PZone->LastTickIntervalMicros(), lwprofile::takeZoneUs(zid), zmobs, zplayers, PZone->getName() });
                    });
                const uint32 entities = players + mobs + npcs;

                // Distribution of per-zone tick intervals (diagnose "why are the top
                // deviations all the same?" — reveals whether it's a plateau/quantum
                // or a smooth spread). Over ticking zones only (interval_us > 0).
                {
                    std::vector<int64> ivals;
                    ivals.reserve(costs.size());
                    for (const auto& c : costs)
                    {
                        if (c.interval_us > 0)
                        {
                            ivals.push_back(c.interval_us);
                        }
                    }
                    if (!ivals.empty())
                    {
                        std::sort(ivals.begin(), ivals.end());
                        int64 sum = 0;
                        for (const auto v : ivals)
                        {
                            sum += v;
                        }
                        ShowInfoFmt("lwdist zi_count={} zi_min_us={} zi_p10_us={} zi_p50_us={} zi_mean_us={} zi_p90_us={} zi_max_us={}",
                                    ivals.size(), ivals.front(), ivals[ivals.size() / 10], ivals[ivals.size() / 2],
                                    sum / static_cast<int64>(ivals.size()), ivals[ivals.size() * 9 / 10], ivals.back());
                    }
                }

                // Per-zone tick interval + AI cost for EVERY real zone (→ off-schedule
                // table, per-zone AI-CPU lines). We emit them all rather than the worst
                // N so the time-series panels have a continuous line per zone instead of
                // a rotating subset. interval_us == 0 means "not measured" (instance zones
                // override ZoneServer, or a zone's very first tick).
                for (const auto& c : costs)
                {
                    ShowInfoFmt("lwzone zone={} name={} interval_us={} ai_us={} mobs={} players={}",
                                c.id, c.name, c.interval_us, c.ai_us, c.mobs, c.players);
                }
                // Per-populated-zone player counts with names → the "where players are" pie chart.
                for (const auto& c : costs)
                {
                    if (c.players > 0)
                    {
                        ShowInfoFmt("lwpop zone={} name={} players={}", c.id, c.name, c.players);
                    }
                }

                // avgGap over the window; busy estimate = how far the probe resumed
                // past the yield (a lower-bound proxy for main-thread work per cadence);
                // headroom = budget minus that busy time.
                const auto avgGapMs   = tickProbeCount_ ? tickGapSumMs_ / static_cast<int64>(tickProbeCount_) : budgetMs;
                const auto busyMs     = std::max<int64>(0, avgGapMs - budgetMs);
                const auto headroomMs = std::max<int64>(0, budgetMs - busyMs);
                const auto headroomPct = budgetMs ? (headroomMs * 100) / budgetMs : 0;

                ShowInfoFmt("living-world: {}/{} zones ticking | {} players | {} mobs | {} npcs | tick {}ms: avg gap {}ms, headroom {}%, {} slips ({} severe)/{}",
                            ticking, total, players, mobs, npcs, budgetMs, avgGapMs, headroomPct, tickOverrunCount_, tickSevereCount_, tickProbeCount_);
                // Machine-readable (parsed by tools/lw_exporter.py):
                ShowInfoFmt("lwmetrics zones_ticking={} zones_total={} players={} mobs={} npcs={} entities={} "
                            "chars_in_combat={} tick_budget_ms={} avg_gap_ms={} max_gap_ms={} busy_ms={} headroom_ms={} headroom_pct={} "
                            "slips={} severe_slips={} probe_ticks={} lua_mem_kb={}",
                            ticking, total, players, mobs, npcs, entities, inCombat,
                            budgetMs, avgGapMs, tickGapMaxMs_, busyMs, headroomMs, headroomPct,
                            tickOverrunCount_, tickSevereCount_, tickProbeCount_,
                            lua.memory_used() / 1024);

                tickProbeCount_   = 0;
                tickOverrunCount_ = 0;
                tickSevereCount_  = 0;
                tickOverrunMaxMs_ = 0;
                tickGapMaxMs_     = 0;
                tickGapSumMs_     = 0;
            });
    }

    zoneutils::TOTDChange(vanadiel_time::get_totd()); // This tells the zones to spawn stuff based on time of day conditions (such as undead at night)

    ShowInfo("do_init: Removing expired database variables");
    uint32 currentTimestamp = earth_time::timestamp();
    db::preparedStmt("DELETE FROM char_vars WHERE expiry > 0 AND expiry <= ?", currentTimestamp);
    db::preparedStmt("DELETE FROM server_variables WHERE expiry > 0 AND expiry <= ?", currentTimestamp);

    moduleutils::OnInit();

    luautils::OnServerStart();

    if (!config_.isTestServer)
    {
        moduleutils::ReportLuaModuleUsage();
    }

    db::enableTimers();

    //
    // Set up the watchdog tasks
    //

    if (!config_.isTestServer)
    {
        scheduler_.postToMainThread(watchdogUpdater());
        scheduler_.postToWorkerThread(watchdogWatcher());
    }

#ifdef TRACY_ENABLE
    ShowInfo("*** TRACY IS ENABLED ***");
#endif // TRACY_ENABLE

    //
    // At this point, the scheduler is loaded up with all the tasks it will need
    // to run everything.
    //

    application_.markLoaded();
}

auto MapEngine::watchdogUpdater() -> Task<void>
{
    // Run "forever"
    while (!scheduler_.closeRequested())
    {
        // If something manages to block the main thread, this task won't be run, and the watcher
        // will kill the server from a worker thread.
        // We do this because if the main thread is blocked severely enough to trigger the watchdog,
        // your server is degraded - likely beyond repair.
        watchdogLastUpdate_ = timer::now();
        co_await scheduler_.yieldFor(kMainThreadBacklogThreshold);
    }
}

auto MapEngine::watchdogWatcher() -> Task<void>
{
    auto period = settings::get<uint32>("main.INACTIVITY_WATCHDOG_PERIOD");

    if (config_.inCI)
    {
        // Double the timer period, to account for the slower CI environment
        period *= 2;
    }

    const auto periodMs = (period > 0) ? std::chrono::milliseconds(period) : 2000ms;

    watchdogLastUpdate_ = timer::now();

    // Run "forever"
    while (!scheduler_.closeRequested())
    {
        const auto lastUpdate = watchdogLastUpdate_.load();
        if ((timer::now() - lastUpdate) >= periodMs)
        {
            if (debug::isRunningUnderDebugger())
            {
                ShowCritical("!!! INACTIVITY WATCHDOG HAS TRIGGERED !!!");
                ShowCriticalFmt("Process main tick has taken {}ms or more.", period);
                ShowCritical("Detaching watchdog thread, it will not fire again until restart.");
                break;
            }
            else if (!settings::get<bool>("main.DISABLE_INACTIVITY_WATCHDOG"))
            {
                std::string outputStr = "!!! INACTIVITY WATCHDOG HAS TRIGGERED !!!\n\n";

                outputStr += fmt::format("Process main tick has taken {}ms or more.\n", period);
                outputStr += fmt::format("Backtrace Messages:\n\n");

                const auto backtrace = logging::GetBacktrace();
                for (const auto& line : backtrace)
                {
                    outputStr += fmt::format("    {}\n", line);
                }

                outputStr += "\nKilling Process!!!\n";

                ShowCritical(outputStr);

                // Allow some time for logging to flush
                std::this_thread::sleep_for(200ms);

                throw std::runtime_error("Watchdog thread time exceeded. Killing process.");
            }
        }

        co_await scheduler_.yieldFor(periodMs);
    }
}

void MapEngine::sessionCleanup() const
{
    TracyZoneScoped;

    networking().sessions().cleanupSessions(networking().ipp());

    zoneutils::ForEachZone(
        [](CZone* PZone)
        {
            PZone->GetZoneEntities()->EraseStaleDynamicTargIDs();
        });
}

void MapEngine::garbageCollect() const
{
    TracyZoneScoped;

    luautils::garbageCollectFull();
}

void MapEngine::onStats(std::vector<std::string>& inputs) const
{
    mapStatistics_->print();
}

void MapEngine::onBacktrace(std::vector<std::string>& inputs) const
{
    const auto backtrace = logging::GetBacktrace();
    for (const auto& line : backtrace)
    {
        fmt::print("{}\n", line);
    }
}

void MapEngine::onReloadRecipes(std::vector<std::string>& inputs) const
{
    fmt::print("> Reloading crafting recipes\n");
    synthutils::LoadSynthRecipes();
}

void MapEngine::onGM(const std::vector<std::string>& inputs) const
{
    if (inputs.size() != 3)
    {
        fmt::print("Usage: gm <char_name> <level>. example: gm Testo 1\n");
        return;
    }

    const auto& name  = inputs[1];
    auto*       PChar = zoneutils::GetCharByName(name);
    if (!PChar)
    {
        fmt::print("Couldnt find character: {}\n", name);
        return;
    }

    const auto level = std::clamp<uint8>(static_cast<uint8>(stoi(inputs[2])), 0, 5);

    PChar->m_GMlevel = level;

    charutils::SaveCharGMLevel(PChar);

    fmt::print("> Promoting {} to GM level {}\n", PChar->name, level);
    PChar->pushPacket<GP_SERV_COMMAND_CHAT_STD>(PChar, MESSAGE_SYSTEM_3, fmt::format("You have been set to GM level {}.", level));
}

auto MapEngine::networking() const -> MapNetworking&
{
    return *networking_;
}

auto MapEngine::statistics() const -> MapStatistics&
{
    return *mapStatistics_;
}

auto MapEngine::zones() const -> std::map<uint16, CZone*>&
{
    return g_PZoneList;
}

auto MapEngine::scheduler() -> Scheduler&
{
    return scheduler_;
}

auto MapEngine::config() const -> MapConfig&
{
    return config_;
}
