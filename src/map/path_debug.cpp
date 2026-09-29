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

#include "path_debug.h"

#include "ai/ai_container.h"
#include "entities/char_entity.h"
#include "entities/mob_entity.h"
#include "enums/chat_message_type.h"
#include "instance.h"
#include "packets/s2c/0x017_chat_std.h"
#include "zone_entities.h"

namespace
{

auto frozenMessage(std::size_t count) -> std::string
{
    return fmt::format("Pathdebug froze {} markers; the mob keeps moving. Use !pathdebug on to resume or !pathdebug off to clear.", count);
}

} // namespace

PathDebug::PathDebug(CZoneEntities& entities)
: entities_(entities)
{
}

auto PathDebug::entities(const CCharEntity& observer) -> CZoneEntities*
{
    if (observer.PInstance)
    {
        return observer.PInstance;
    }
    return observer.loc.zone ? observer.loc.zone->GetZoneEntities() : nullptr;
}

auto PathDebug::validTarget(const CCharEntity& observer, const Watch& watch) const -> CBaseEntity*
{
    auto* target = entities_.GetEntity(watch.target.ActIndex, TYPE_MOB);
    if (entities(observer) != &entities_ || !target || target->id != watch.target.UniqueNo ||
        target->serial() != watch.target.serial || target->PInstance != observer.PInstance ||
        (target->status != xi::Status::Normal && target->status != xi::Status::Update) ||
        !target->PAI || !target->PAI->PathFind || static_cast<CMobEntity*>(target)->isDead())
    {
        return nullptr;
    }
    return target;
}

auto PathDebug::watch(CCharEntity& observer, CBaseEntity& target, bool frozen, uint16 model) -> std::string
{
    Watch candidate;
    candidate.observer = observer.entityId();
    candidate.target   = target.entityId();
    candidate.model    = model;
    if (observer.m_GMlevel == 0 || target.objtype != TYPE_MOB || !validTarget(observer, candidate))
    {
        return "Select a living mob in your zone and instance.";
    }

    // Retarget and repeated toggles reuse the existing reservation.
    auto* reserved = reserve(observer, MarkerLimit);
    if (!reserved)
    {
        return "Not enough dynamic entity IDs for pathdebug; try again after 60 seconds.";
    }
    auto& watch  = *reserved;
    watch.target = candidate.target;
    watch.model  = model;
    watch.points.clear();
    for (auto& slot : watch.slots)
    {
        slot.point.reset();
    }

    const auto now   = timer::now();
    const auto count = refresh(observer, watch, target.PAI->PathFind->GetDebugSnapshot(now));
    watch.nextUpdate = now + 250ms;
    watch.mode       = frozen ? (count ? Mode::Frozen : Mode::WaitingForPath) : Mode::Live;
    if (watch.mode == Mode::WaitingForPath)
    {
        return "Pathdebug waiting for the mob's next path, then freezing its markers. The mob keeps moving. Use !pathdebug off to cancel.";
    }
    return frozen ? frozenMessage(count) : "Pathdebug watching: NEXT, numbered waypoints, End and Dest. Use !pathdebug off to stop.";
}

auto PathDebug::show(CCharEntity& observer, std::vector<PathDebugMarker> points) -> std::string
{
    if (observer.m_GMlevel == 0 || entities(observer) != &entities_)
    {
        return "Pathdebug requires GM permission in this zone.";
    }
    if (points.empty())
    {
        return "Pathdebug needs at least one point.";
    }
    // debug tooling: take every free dynamic ID the point set could use
    entities_.EraseStaleDynamicTargIDs();
    const auto it    = watches_.find(observer.serial());
    const auto held  = it == watches_.end() ? 0 : it->second.slots.size();
    const auto free  = 0x200 - entities_.GetUsedDynamicTargIDsCount();
    const auto count = std::min(points.size(), held + free);
    auto*      watch = count ? reserve(observer, count) : nullptr;
    if (!watch)
    {
        return "Not enough dynamic entity IDs for pathdebug; try again after 60 seconds.";
    }
    const auto loaded = points.size();
    watch->target     = {};
    watch->mode       = Mode::Points;
    watch->points     = std::move(points);
    for (auto& slot : watch->slots)
    {
        slot.point.reset();
    }
    watch->nextUpdate = timer::now() + 250ms;
    refreshPoints(observer, *watch);
    return fmt::format("Pathdebug loaded {} points; showing up to {} within {}y. Use !pathdebug off to clear.", loaded, watch->slots.size(), PointRange);
}

void PathDebug::refreshPoints(CCharEntity& observer, Watch& watch)
{
    std::vector<std::pair<float, std::size_t>> nearby;
    for (std::size_t i = 0; i < watch.points.size(); ++i)
    {
        if (const auto dist = distance(observer.loc.p, watch.points[i].position); dist <= PointRange)
        {
            nearby.emplace_back(dist, i);
        }
    }
    if (nearby.size() > watch.slots.size())
    {
        std::ranges::nth_element(nearby, nearby.begin() + watch.slots.size());
        nearby.resize(watch.slots.size());
    }
    std::vector<bool> wanted(watch.points.size());
    for (const auto& [dist, index] : nearby)
    {
        wanted[index] = true;
    }

    // a point keeps its slot while in range, so the client never sees it jump between entities
    std::vector<bool> shown(watch.points.size());
    for (auto& slot : watch.slots)
    {
        if (slot.point && !wanted[*slot.point])
        {
            slot.point.reset();
        }
        if (slot.point)
        {
            shown[*slot.point] = true;
        }
    }
    auto freeSlot = watch.slots.begin();
    for (const auto& [dist, index] : nearby)
    {
        if (!shown[index])
        {
            freeSlot        = std::ranges::find_if(freeSlot, watch.slots.end(), [](const Slot& slot)
                                                   {
                                                return !slot.point;
                                                   });
            freeSlot->point = index;
        }
    }

    for (auto& slot : watch.slots)
    {
        const auto next = slot.point ? std::make_optional(watch.points[*slot.point]) : std::nullopt;
        if (next != slot.marker)
        {
            slot.marker = next;
            send(observer, slot);
        }
    }
}

auto PathDebug::reserve(CCharEntity& observer, std::size_t count) -> Watch*
{
    auto& watch    = watches_[observer.serial()];
    watch.observer = observer.entityId();
    if (watch.slots.size() < count)
    {
        const auto ids = entities_.ReserveClientEntityIds(count - watch.slots.size());
        if (ids.empty())
        {
            if (watch.slots.empty())
            {
                watches_.erase(observer.serial());
            }
            return nullptr;
        }
        for (auto targid : ids)
        {
            watch.slots.push_back({ targid, std::nullopt, std::nullopt });
        }
    }
    return &watch;
}

void PathDebug::send(CCharEntity& observer, const Slot& slot)
{
    const auto id = entities_.DynamicEntityLongId(slot.targid);
    observer.queueClientEntityPacket(id, std::make_unique<CPathDebugPacket>(id, slot.targid, slot.marker ? &*slot.marker : nullptr));
}

void PathDebug::stop(CCharEntity& observer, bool sendDespawn)
{
    const auto it = watches_.find(observer.serial());
    if (it == watches_.end())
    {
        return;
    }
    // A late cleanup must never send old-instance IDs into the observer's new context.
    sendDespawn = sendDespawn && entities(observer) == &entities_;
    for (auto& slot : it->second.slots)
    {
        // Remove pending spawns/updates even on zone-out, before releasing the IDs.
        observer.queueClientEntityPacket(entities_.DynamicEntityLongId(slot.targid), nullptr);
        if (sendDespawn)
        {
            slot.marker.reset();
            send(observer, slot);
        }
        entities_.ReleaseClientEntityId(slot.targid);
    }
    watches_.erase(it);
}

void PathDebug::tick(CCharEntity& observer, timer::time_point tick)
{
    auto it = watches_.find(observer.serial());
    if (it == watches_.end())
    {
        return;
    }
    if (observer.status == xi::Status::Shutdown || observer.requestedZoneChange || observer.requestedWarp != WarpRequest::None)
    {
        stop(observer, false);
        return;
    }
    auto& watch = it->second;
    if (observer.m_GMlevel == 0)
    {
        stop(observer);
        return;
    }
    if (watch.mode == Mode::Points)
    {
        if (tick >= watch.nextUpdate)
        {
            refreshPoints(observer, watch);
            watch.nextUpdate = tick + 250ms;
        }
        return;
    }
    auto* target = validTarget(observer, watch);
    if (!target)
    {
        stop(observer);
        return;
    }
    if (tick >= watch.nextUpdate && watch.mode != Mode::Frozen)
    {
        const auto count = refresh(observer, watch, target->PAI->PathFind->GetDebugSnapshot(tick));
        watch.nextUpdate = tick + 250ms;
        if (watch.mode == Mode::WaitingForPath && count > 0)
        {
            watch.mode = Mode::Frozen;
            observer.pushPacket<GP_SERV_COMMAND_CHAT_STD>(&observer, MESSAGE_SYSTEM_1, frozenMessage(count));
        }
    }
}

void PathDebug::targetGone(const CBaseEntity& target)
{
    for (auto it = watches_.begin(); it != watches_.end();)
    {
        const auto& watch = (it++)->second;
        if (watch.mode != Mode::Points && watch.target.serial == target.serial())
        {
            if (auto* observer = entities_.GetCharByID(watch.observer.UniqueNo); observer && observer->serial() == watch.observer.serial)
            {
                stop(*observer);
            }
        }
    }
}

auto PathDebug::request(CCharEntity& observer, uint16 targid, uint32 id) -> bool
{
    if (id != 0)
    {
        const auto low = id & 0xFFF;
        if (low < 0x800 || low > 0x9FF)
        {
            return false;
        }
        targid = static_cast<uint16>(low - 0x100);
        if (entities_.DynamicEntityLongId(targid) != id)
        {
            return false;
        }
    }
    if (!entities_.IsClientEntityId(targid))
    {
        return false;
    }

    // Includes quarantined IDs and other observers' IDs; neither may resolve as real entities.
    tick(observer, timer::now());
    if (auto it = watches_.find(observer.serial()); it != watches_.end())
    {
        for (const auto& slot : it->second.slots)
        {
            if (slot.targid == targid)
            {
                send(observer, slot);
                break;
            }
        }
    }
    return true;
}

auto PathDebug::refresh(CCharEntity& observer, Watch& watch, const CPathFind::DebugSnapshot& snapshot) -> std::size_t
{
    const auto desired = markers(snapshot, watch.model);
    refresh(observer, watch, desired);
    return desired.size();
}

void PathDebug::refresh(CCharEntity& observer, Watch& watch, const std::vector<PathDebugMarker>& desired)
{
    for (std::size_t i = 0; i < watch.slots.size(); ++i)
    {
        const std::optional<PathDebugMarker> next = i < desired.size() ? std::make_optional(desired[i]) : std::nullopt;
        auto&                                slot = watch.slots[i];
        if (next != slot.marker)
        {
            slot.marker = next;
            send(observer, slot);
        }
    }
}

auto PathDebug::markers(const CPathFind::DebugSnapshot& snapshot, uint16 model) -> std::vector<PathDebugMarker>
{
    std::vector<PathDebugMarker> result;
    if (snapshot.cursor >= snapshot.points.size())
    {
        return result;
    }
    const auto markerModel = model ? model : PathDebugMarker::DefaultModel;
    const auto end         = std::min(snapshot.points.size(), snapshot.cursor + WaypointLimit);
    for (auto i = snapshot.cursor; i < end; ++i)
    {
        const bool next = i == snapshot.cursor;
        auto       name = fmt::format("{}{}", next ? "NEXT" : "", i + 1);
        if (next && (snapshot.paused || snapshot.waiting))
        {
            name += snapshot.paused ? " P" : " W";
        }
        result.push_back({ snapshot.points[i].position, markerModel, std::move(name) });
    }
    const auto& endpoint = snapshot.points.back().position;
    const bool  same     = endpoint.x == snapshot.destination.x && endpoint.y == snapshot.destination.y && endpoint.z == snapshot.destination.z;
    if (end == snapshot.points.size())
    {
        // Avoid two labels occupying exactly the same final waypoint.
        result.back().name += same ? " E+D" : " End";
    }
    else
    {
        result.push_back({ endpoint, markerModel, fmt::format("{} {}", same ? "End+Dest" : "End", snapshot.points.size()) });
    }
    if (!same)
    {
        result.push_back({ snapshot.destination, markerModel, "Dest" });
    }
    return result;
}
