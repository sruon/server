/*
Living-world AI profiling: cheap accumulators for mob-AI hot paths (line-of-sight,
roaming, navmesh pathfinding). A scoped timer adds the elapsed wall-time + a call
count into an atomic pair; flushStatistics drains them each stats interval and logs
an `lwai` line for the Prometheus exporter. Mob AI runs on the main thread so the
atomics are uncontended; they're atomic only to be safe if that ever changes.
*/
#pragma once

#include <atomic>
#include <chrono>

#include "common/cbasetypes.h"

namespace lwprofile
{
    struct Accum
    {
        std::atomic<int64> us{ 0 };
        std::atomic<int64> calls{ 0 };
    };

    inline Accum los;  // CBaseEntity::CanSeeTarget (ray-intersect line of sight)
    inline Accum roam; // CMobController::DoRoamTick
    inline Accum path; // CNavMesh::findPath
    // Time INSIDE LuaJIT, split by per-tick hook (each = us + call count).
    inline Accum lua_roam;
    inline Accum lua_roamaction;
    inline Accum lua_fight;
    inline Accum lua_path;
    inline Accum lua_effect;

    // AI action states (synchronous ::Update) — how long the AI spends processing
    // each action type (attack round, magic casting, ability, weaponskill, ...).
    inline Accum st_attack;
    inline Accum st_magic;
    inline Accum st_ability;
    inline Accum st_weaponskill;
    inline Accum st_ranged;
    inline Accum st_item;

    // Per-zone AI CPU time (µs), attributed by the entity's zone. Because these
    // hot paths are synchronous, this is REAL per-zone cost (unlike wall-timing a
    // zone's interleaved coroutine tick). Read+reset by the living-world heartbeat.
    constexpr int      kMaxZone = 1024;
    inline std::atomic<int64> zoneUs[kMaxZone];

    // RAII scoped timer: accumulate into the global bucket, and (if a zone is
    // given) into that zone's per-zone bucket, on destruction.
    struct Scope
    {
        Accum&                                a;
        uint16                                zone;
        std::chrono::steady_clock::time_point t0;
        explicit Scope(Accum& acc, uint16 z = 0)
        : a(acc)
        , zone(z)
        , t0(std::chrono::steady_clock::now())
        {
        }
        ~Scope()
        {
            const auto us = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - t0).count();
            a.us.fetch_add(us, std::memory_order_relaxed);
            a.calls.fetch_add(1, std::memory_order_relaxed);
            if (zone > 0 && zone < kMaxZone)
            {
                zoneUs[zone].fetch_add(us, std::memory_order_relaxed);
            }
        }
    };

    inline int64 takeZoneUs(uint16 z)
    {
        return z < kMaxZone ? zoneUs[z].exchange(0, std::memory_order_relaxed) : 0;
    }

    // Inbound network (the map only tracks sent bytes in MapStatistics, not received).
    inline std::atomic<int64> recvBytes{ 0 };
    inline std::atomic<int64> recvPkts{ 0 };

    // Read-and-reset (called once per stats interval).
    inline int64 takeUs(Accum& a)
    {
        return a.us.exchange(0, std::memory_order_relaxed);
    }
    inline int64 takeCalls(Accum& a)
    {
        return a.calls.exchange(0, std::memory_order_relaxed);
    }
} // namespace lwprofile
