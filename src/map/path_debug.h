/*
===========================================================================

  Copyright (c) 2026 LandSandBoat Dev Teams

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

#include "ai/helpers/pathfind/pathfind.h"
#include "entities/entity_id.h"
#include "packets/path_debug.h"

#include <map>
#include <optional>

class CCharEntity;
class CZoneEntities;

// Owned by the same zone/instance collection that reserves the marker IDs.
class PathDebug
{
public:
    static constexpr std::size_t WaypointLimit = 16;
    static constexpr std::size_t MarkerLimit   = WaypointLimit + 2;
    static constexpr float       PointRange    = 50.0f;

    explicit PathDebug(CZoneEntities& entities);

    auto watch(CCharEntity& observer, CBaseEntity& target, bool frozen, uint16 model) -> std::string;
    auto show(CCharEntity& observer, std::vector<PathDebugMarker> points) -> std::string;
    void stop(CCharEntity& observer, bool sendDespawn = true);
    void tick(CCharEntity& observer, timer::time_point tick);
    void targetGone(const CBaseEntity& target);
    auto request(CCharEntity& observer, uint16 targid, uint32 id = 0) -> bool;

    static auto markers(const CPathFind::DebugSnapshot& snapshot, uint16 model = 0) -> std::vector<PathDebugMarker>;
    static auto entities(const CCharEntity& observer) -> CZoneEntities*;

private:
    enum class Mode
    {
        Live,
        WaitingForPath,
        Frozen,
        Points, // caller-supplied markers, no target
    };

    struct Slot
    {
        uint16                         targid;
        std::optional<PathDebugMarker> marker;
        std::optional<std::size_t>     point; // index into Watch::points; pinned while in range
    };

    struct Watch
    {
        EntityId                     observer;
        EntityId                     target;
        std::vector<Slot>            slots;
        std::vector<PathDebugMarker> points;
        Mode                         mode{ Mode::Live };
        uint16                       model{ 0 };
        timer::time_point            nextUpdate{ timer::time_point::min() };
    };

    auto validTarget(const CCharEntity& observer, const Watch& watch) const -> CBaseEntity*;
    auto reserve(CCharEntity& observer, std::size_t count) -> Watch*;
    auto refresh(CCharEntity& observer, Watch& watch, const CPathFind::DebugSnapshot& snapshot) -> std::size_t;
    void refresh(CCharEntity& observer, Watch& watch, const std::vector<PathDebugMarker>& desired);
    void refreshPoints(CCharEntity& observer, Watch& watch);
    void send(CCharEntity& observer, const Slot& slot);

    CZoneEntities&          entities_;
    std::map<uint64, Watch> watches_;
};
