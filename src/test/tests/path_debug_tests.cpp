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

#include "common/database.h"
#include "common/scheduler.h"
#include "map/ai/ai_container.h"
#include "map/entities/char_entity.h"
#include "map/instance.h"
#include "map/packets/c2s/0x016_charreq.h"
#include "map/packets/c2s/0x017_charreq2.h"
#include "map/packets/entity_update.h"
#include "map/path_debug.h"
#include "map/zone_entities.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

namespace
{

// Keep entity/zone constructors and character history teardown away from a live database.
class EmptyDatabase final : public db::Database
{
public:
    EmptyDatabase()
    {
        db::setDatabase(this);
    }

    ~EmptyDatabase() override
    {
        db::setDatabase(nullptr);
    }

    auto execute(const std::string&, const std::vector<db::BoundValue>&) -> std::unique_ptr<db::ResultSet> override
    {
        return nullptr;
    }

    auto executeBulk(const std::string&, const std::vector<db::BoundValue>&) -> std::unique_ptr<db::ResultSet> override
    {
        return nullptr;
    }

    auto getSchema() -> std::string override
    {
        return {};
    }

    auto getVersion() -> std::string override
    {
        return {};
    }

    auto getDriverVersion() -> std::string override
    {
        return {};
    }
};

} // namespace

TEST_CASE("pathdebug watches stay private, reuse IDs, and clean up on lifecycle changes", "[pathdebug]")
{
    EmptyDatabase database;
    Scheduler     scheduler(1);
    CCharEntity   observer;
    CCharEntity   other;
    CZone         zone(scheduler, {}, xi::ZoneId::WestRonfaure, REGION_TYPE::RONFAURE, CONTINENT_TYPE::THE_MIDDLE_LANDS, 0, std::nullopt);
    auto&         entities = *zone.GetZoneEntities();
    auto&         debug    = entities.GetPathDebug();
    auto*         mob      = new CMobEntity(); // zone owns inserted mobs
    mob->targid            = 1;
    mob->id                = 0x01064001;
    mob->health.hp         = 100;
    mob->status            = xi::Status::Update;
    entities.InsertMOB(mob);
    observer.id        = 100;
    observer.targid    = 0x400;
    observer.health.hp = 1;
    observer.m_GMlevel = 1;
    entities.InsertPC(&observer);
    other.id        = 101;
    other.targid    = 0x401;
    other.health.hp = 1;
    other.m_GMlevel = 1;
    entities.InsertPC(&other);
    observer.clearPacketList();
    other.clearPacketList();
    REQUIRE(mob->PAI->PathFind->PathThrough({ { { 5, -12, 0, 0, 0 }, 0s, false }, { { 25, -22, 5, 0, 0 }, 0s, false } }));
    debug.watch(observer, *mob, false, 0);
    const auto first = observer.getPacketListCopy();
    REQUIRE(first.size() == 2);
    const auto markerId     = first.front()->ref<uint32>(0x04);
    const auto markerTargid = first.front()->ref<uint16>(0x08);
    CHECK(entities.GetEntity(markerTargid) == nullptr);
    CHECK(other.isPacketListEmpty());
    CHECK(observer.SpawnNPCList.empty());
    observer.clearPacketList();
    debug.tick(observer, timer::now() + 1s);
    CHECK(observer.isPacketListEmpty());
    debug.watch(observer, *mob, false, 0);
    CHECK(entities.GetUsedDynamicTargIDsCount() == PathDebug::MarkerLimit);

    SECTION("both entity information requests resend only the observer's marker")
    {
        GP_CLI_COMMAND_CHARREQ request{};
        request.ActIndex = markerTargid;
        request.process(nullptr, &observer);
        REQUIRE(observer.getPacketCount() == 1);
        CHECK(observer.popPacket()->ref<uint32>(0x04) == markerId);
        GP_CLI_COMMAND_CHARREQ2 request2{};
        request2.UniqueNo2 = markerId;
        request2.UniqueNo3 = markerId;
        request2.ActIndex  = markerTargid;
        request2.process(nullptr, &observer);
        CHECK(observer.getPacketCount() == 1);
        request2.process(nullptr, &other);
        CHECK(other.isPacketListEmpty());
    }
    SECTION("idle clears markers and off preserves pending despawns")
    {
        mob->PAI->PathFind->Clear();
        debug.tick(observer, timer::now() + 2s);
        REQUIRE(observer.getPacketCount() == 2);
        CHECK(observer.popPacket()->ref<uint8>(0x0A) == 0x30);
        debug.stop(observer);
        for (const auto& packet : observer.getPacketListCopy())
        {
            CHECK(packet->ref<uint8>(0x0A) == 0x30);
        }
        CHECK(entities.IsClientEntityId(markerTargid));
    }
    SECTION("frozen snapshot survives route changes but ends when target disappears")
    {
        CHECK(debug.watch(observer, *mob, true, 0).starts_with("Pathdebug froze 2 markers"));
        observer.clearPacketList();
        mob->PAI->PathFind->Clear();
        debug.tick(observer, timer::now() + 2s);
        CHECK(observer.isPacketListEmpty());
        CHECK(debug.request(observer, markerTargid));
        REQUIRE(observer.getPacketCount() == 1);
        CHECK(observer.popPacket()->ref<float>(0x0C) == 5);
        mob->FadeOut();
        CHECK(observer.getPacketCount() == PathDebug::MarkerLimit);
        CHECK(observer.popPacket()->ref<uint8>(0x0A) == 0x30);
    }
    SECTION("freezing an idle mob waits for a nonempty path")
    {
        mob->PAI->PathFind->Clear();
        CHECK(debug.watch(observer, *mob, true, 51).starts_with("Pathdebug waiting"));
        observer.clearPacketList();
        debug.tick(observer, timer::now() + 2s);
        CHECK(observer.isPacketListEmpty());

        SECTION("captures once, reports the count, and keeps the snapshot until resumed")
        {
            REQUIRE(mob->PAI->PathFind->PathThrough({ { { 35, -2, 10, 0, 0 }, 0s, false }, { { 45, -4, 10, 0, 0 }, 0s, false } }));
            debug.tick(observer, timer::now() + 3s);
            REQUIRE(observer.getPacketCount() == 3);
            auto packet = observer.popPacket();
            CHECK(packet->getType() == 0x0E);
            CHECK(packet->ref<float>(0x0C) == 35);
            CHECK(packet->ref<float>(0x10) == -3);
            CHECK(packet->ref<uint16>(0x32) == 51);
            packet = observer.popPacket();
            CHECK(packet->getType() == 0x0E);
            CHECK(packet->ref<float>(0x0C) == 45);
            packet = observer.popPacket();
            CHECK(packet->getType() == 0x17);
            CHECK(std::string(&packet->ref<char>(0x17)).starts_with("Pathdebug froze 2 markers"));
            CHECK(other.isPacketListEmpty());
            CHECK(entities.GetUsedDynamicTargIDsCount() == PathDebug::MarkerLimit);

            mob->PAI->PathFind->Clear();
            debug.tick(observer, timer::now() + 4s);
            CHECK(observer.isPacketListEmpty());
            CHECK(debug.request(observer, markerTargid));
            REQUIRE(observer.getPacketCount() == 1);
            CHECK(observer.popPacket()->ref<float>(0x0C) == 35);

            CHECK(debug.watch(observer, *mob, false, 0).starts_with("Pathdebug watching"));
            REQUIRE(observer.getPacketCount() == 2);
            CHECK(observer.popPacket()->ref<uint8>(0x0A) == 0x30);
            CHECK(observer.popPacket()->ref<uint8>(0x0A) == 0x30);
        }
        SECTION("off cancels a pending capture")
        {
            debug.stop(observer);
            observer.clearPacketList();
            REQUIRE(mob->PAI->PathFind->PathThrough({ { { 35, -2, 10, 0, 0 }, 0s, false } }));
            debug.tick(observer, timer::now() + 3s);
            CHECK(observer.isPacketListEmpty());
        }
        SECTION("resuming live updates cancels a pending capture")
        {
            CHECK(debug.watch(observer, *mob, false, 0).starts_with("Pathdebug watching"));
            REQUIRE(mob->PAI->PathFind->PathThrough({ { { 35, -2, 10, 0, 0 }, 0s, false } }));
            debug.tick(observer, timer::now() + 3s);
            REQUIRE(observer.getPacketCount() == 1);
            CHECK(observer.popPacket()->ref<float>(0x0C) == 35);
            mob->PAI->PathFind->Clear();
            debug.tick(observer, timer::now() + 4s);
            REQUIRE(observer.getPacketCount() == 1);
            CHECK(observer.popPacket()->ref<uint8>(0x0A) == 0x30);
        }
    }
    SECTION("death stops watching")
    {
        mob->health.hp = 0;
        debug.tick(observer, timer::now() + 1s);
        CHECK(observer.getPacketCount() == PathDebug::MarkerLimit);
        CHECK(observer.popPacket()->ref<uint8>(0x0A) == 0x30);
    }
    SECTION("two observers own separate pools")
    {
        debug.watch(other, *mob, false, 0);
        CHECK(entities.GetUsedDynamicTargIDsCount() == 2 * PathDebug::MarkerLimit);
        REQUIRE(other.getPacketCount() == 2);
        CHECK(other.popPacket()->ref<uint32>(0x04) != markerId);
        debug.stop(observer);
        CHECK(other.getPacketCount() == 1);
    }
    SECTION("zone-out removes queued spawns and despawns")
    {
        debug.request(observer, markerTargid);
        observer.requestedZoneChange = true;
        debug.tick(observer, timer::now());
        CHECK(observer.isPacketListEmpty());
        CHECK(entities.IsClientEntityId(markerTargid));
    }
    SECTION("an observer context change cannot send stale despawns")
    {
        debug.request(observer, markerTargid);
        observer.loc.zone = nullptr;
        debug.tick(observer, timer::now());
        CHECK(observer.isPacketListEmpty());
        CHECK(entities.IsClientEntityId(markerTargid));
        observer.loc.zone = &zone;
    }
    SECTION("an exhausted allocator leaves no partial watch")
    {
        REQUIRE(entities.ReserveClientEntityIds(0x200 - PathDebug::MarkerLimit).size() == 0x200 - PathDebug::MarkerLimit);
        CHECK(debug.watch(other, *mob, false, 0).starts_with("Not enough"));
        CHECK(other.isPacketListEmpty());
        CHECK(entities.GetUsedDynamicTargIDsCount() == 0x200);
    }
    SECTION("replacing an entity with the same IDs does not switch the watch")
    {
        auto* replacement      = new CMobEntity();
        replacement->targid    = mob->targid;
        replacement->id        = mob->id;
        replacement->health.hp = 100;
        replacement->status    = xi::Status::Update;
        entities.InsertMOB(replacement);
        destroy(mob);
        observer.clearPacketList();
        debug.tick(observer, timer::now() + 1s);
        CHECK(observer.getPacketCount() == PathDebug::MarkerLimit);
        CHECK(observer.popPacket()->ref<uint8>(0x0A) == 0x30);
    }
    SECTION("normal entities skip reserved marker IDs")
    {
        CMobEntity dynamic;
        entities.AssignDynamicTargIDandLongID(&dynamic);
        CHECK(dynamic.targid == markerTargid + PathDebug::MarkerLimit);
        CHECK(dynamic.id == entities.DynamicEntityLongId(dynamic.targid));
        CHECK_FALSE(entities.IsClientEntityId(dynamic.targid));
    }
    SECTION("expired reservations purge unsent despawns before reuse")
    {
        debug.stop(observer);
        REQUIRE(observer.getPacketCount() == PathDebug::MarkerLimit);
        const auto oldOffset = timer::time_offset;
        timer::add_offset(61s);
        entities.EraseStaleDynamicTargIDs();
        timer::time_offset = oldOffset;
        CHECK(observer.isPacketListEmpty());
        CHECK(entities.GetUsedDynamicTargIDsCount() == 0);
    }
}

TEST_CASE("pathdebug reserves and releases through the observer's instance", "[pathdebug]")
{
    EmptyDatabase database;
    Scheduler     scheduler(1);
    CCharEntity   observer;
    CZone         zone(scheduler, {}, xi::ZoneId::WestRonfaure, REGION_TYPE::RONFAURE, CONTINENT_TYPE::THE_MIDDLE_LANDS, 0, std::nullopt);
    CInstance     instance(scheduler, {}, &zone, 1);
    auto*         mob = new CMobEntity();
    mob->targid       = 1;
    mob->id           = 0x01064001;
    mob->health.hp    = 100;
    mob->status       = xi::Status::Update;
    mob->PInstance    = &instance;
    instance.InsertMOB(mob);
    observer.id        = 100;
    observer.targid    = 0x400;
    observer.health.hp = 1;
    observer.m_GMlevel = 1;
    observer.PInstance = &instance;
    instance.InsertPC(&observer);
    auto& debug = instance.GetPathDebug();
    CHECK(PathDebug::entities(observer) == &instance);
    CHECK(debug.watch(observer, *mob, false, 0).starts_with("Pathdebug watching"));
    CHECK(instance.GetUsedDynamicTargIDsCount() == PathDebug::MarkerLimit);
    CHECK(zone.GetZoneEntities()->GetUsedDynamicTargIDsCount() == 0);
    debug.stop(observer, false);
    const auto oldOffset = timer::time_offset;
    timer::add_offset(61s);
    instance.EraseStaleDynamicTargIDs();
    timer::time_offset = oldOffset;
    CHECK(instance.GetUsedDynamicTargIDsCount() == 0);
    observer.PInstance = nullptr;
}

TEST_CASE("pathdebug shows arbitrary points and grows the reservation", "[pathdebug]")
{
    EmptyDatabase database;
    Scheduler     scheduler(1);
    CCharEntity   observer;
    CZone         zone(scheduler, {}, xi::ZoneId::WestRonfaure, REGION_TYPE::RONFAURE, CONTINENT_TYPE::THE_MIDDLE_LANDS, 0, std::nullopt);
    auto&         entities = *zone.GetZoneEntities();
    auto&         debug    = entities.GetPathDebug();
    auto*         mob      = new CMobEntity();
    mob->targid            = 1;
    mob->id                = 0x01064001;
    mob->health.hp         = 100;
    mob->status            = xi::Status::Update;
    entities.InsertMOB(mob);
    observer.id        = 100;
    observer.targid    = 0x400;
    observer.health.hp = 1;
    observer.m_GMlevel = 1;
    entities.InsertPC(&observer);
    observer.clearPacketList();

    CHECK(debug.show(observer, {}).starts_with("Pathdebug needs"));
    CHECK(entities.GetUsedDynamicTargIDsCount() == 0);

    // one near point, one out of range
    CHECK(debug.show(observer, { { { 1, 2, 3, 0, 0 }, 51, "a" }, { { 100, 0, 0, 0, 0 }, 51, "far" } }).starts_with("Pathdebug loaded 2"));
    CHECK(entities.GetUsedDynamicTargIDsCount() == 2);
    REQUIRE(observer.getPacketCount() == 1);
    auto packet = observer.popPacket();
    CHECK(packet->ref<float>(0x0C) == 1);
    CHECK(packet->ref<uint16>(0x32) == 51);
    CHECK(std::string(&packet->ref<char>(0x34)) == "a");

    // walking over swaps which point is shown
    observer.loc.p = { 90, 0, 0, 0, 0 };
    debug.tick(observer, timer::now() + 5s);
    REQUIRE(observer.getPacketCount() == 1);
    packet = observer.popPacket();
    CHECK(packet->ref<float>(0x0C) == 100);
    CHECK(std::string(&packet->ref<char>(0x34)) == "far");

    // a point entering range must not shift the points already shown onto other IDs
    observer.loc.p = { 0, 0, 0, 0, 0 };
    observer.clearPacketList();
    CHECK(debug.show(observer, { { { 20, 0, 0, 0, 0 }, 51, "a" }, { { 60, 0, 0, 0, 0 }, 51, "b" }, { { 40, 0, 0, 0, 0 }, 51, "c" } }).starts_with("Pathdebug loaded 3"));
    REQUIRE(observer.getPacketCount() == 2); // a and c; b is out of range
    observer.clearPacketList();
    observer.loc.p = { 15, 0, 0, 0, 0 };
    debug.tick(observer, timer::now() + 10s);
    REQUIRE(observer.getPacketCount() == 1);
    packet = observer.popPacket();
    CHECK(std::string(&packet->ref<char>(0x34)) == "b");
    observer.loc.p = { 90, 0, 0, 0, 0 };

    // grows to the whole dynamic range, then shows only the nearest
    std::vector<PathDebugMarker> many;
    for (int i = 0; i < 600; ++i)
    {
        many.push_back({ { 90.0f + static_cast<float>(i) * 0.01f, 0, 0, 0, 0 }, 51, std::to_string(i) });
    }
    observer.clearPacketList();
    CHECK(debug.show(observer, many).starts_with("Pathdebug loaded 600 points; showing up to 512"));
    CHECK(entities.GetUsedDynamicTargIDsCount() == 0x200);
    CHECK(observer.getPacketCount() == 0x200);
    CHECK(observer.getPacketListCopy().back()->ref<float>(0x0C) == Catch::Approx(90.0f + 511 * 0.01f));
    CHECK(debug.show(observer, many).starts_with("Pathdebug loaded 600"));
    observer.clearPacketList();

    REQUIRE(mob->PAI->PathFind->PathThrough({ { { 5, -12, 0, 0, 0 }, 0s, false } }));
    CHECK(debug.watch(observer, *mob, false, 0).starts_with("Pathdebug watching"));
    CHECK(entities.GetUsedDynamicTargIDsCount() == 0x200);
    CHECK(observer.getPacketCount() == 0x200);

    debug.stop(observer, false);
}

TEST_CASE("pathdebug serializes private fixed-model markers", "[pathdebug]")
{
    const PathDebugMarker marker{ { 12.5f, -42.25f, 8.75f, 0, 0 }, PathDebugMarker::DefaultModel, "NEXT 2 paused" };
    CPathDebugPacket      packet(0x01064800, 0x700, &marker);
    CHECK(packet.getType() == 0x0E);
    CHECK(packet.getSize() == 0x48);
    CHECK(packet.ref<uint32>(0x04) == 0x01064800);
    CHECK(packet.ref<uint16>(0x08) == 0x700);
    CHECK(packet.ref<uint8>(0x0A) == 0x1F);
    CHECK(packet.ref<float>(0x0C) == 12.5f);
    CHECK(packet.ref<float>(0x10) == -43.25f); // lift the display without changing the path
    CHECK(marker.position.y == -42.25f);
    CHECK(packet.ref<float>(0x14) == 8.75f);
    CHECK(packet.ref<uint32>(0x18) == 0x00008000);                // GroundFlag set; no walking time or facing target
    CHECK(packet.ref<uint16>(0x1C) == 0);                         // no movement/animation speed
    CHECK((packet.ref<uint32>(0x20) & 0x00080100) == 0x00080100); // target off, initialize position
    CHECK((packet.ref<uint8>(0x28) & 0x40) == 0);                 // no interaction
    CHECK(packet.ref<uint8>(0x2B) == 0x10);                       // no collision, name visible
    CHECK(packet.ref<uint32>(0x2C) == 0);                         // no claim
    CHECK(packet.ref<uint16>(0x30) == MODEL_STANDARD);
    CHECK(packet.ref<uint16>(0x32) == 1525);
    CHECK(std::string(&packet.ref<char>(0x34)) == marker.name);

    CPathDebugPacket despawn(0x01064800, 0x700, nullptr);
    CHECK(despawn.ref<uint8>(0x0A) == 0x30);
    CHECK(despawn.ref<uint8>(0x1F) == 2);
    CHECK(despawn.ref<uint32>(0x04) == packet.ref<uint32>(0x04));
}

TEST_CASE("pathdebug bounds waypoints and preserves cursor, height and eventual destination", "[pathdebug]")
{
    CPathFind::DebugSnapshot snapshot;
    for (int i = 0; i < 40; ++i)
    {
        snapshot.points.push_back({ { static_cast<float>(i), -15.5f, 1.0f, 0, 0 }, 0s, false });
    }
    snapshot.cursor      = 7;
    snapshot.destination = { 100, -25, 4, 0, 0 };
    snapshot.partial     = true;
    snapshot.paused      = true;
    auto markers         = PathDebug::markers(snapshot);
    REQUIRE(markers.size() == PathDebug::MarkerLimit);
    CHECK(markers.front().name == "NEXT8 P");
    CHECK(markers.front().position.x == 7);
    CHECK(markers.front().position.y == -15.5f);
    for (const auto& marker : markers)
    {
        CHECK(marker.model == 1525);
    }
    CHECK(markers[15].name == "23");
    CHECK(markers[16].name == "End 40");
    CHECK(markers[16].position.x == 39);
    CHECK(markers[17].name == "Dest");
    CHECK(markers[17].position.x == 100);
    CHECK(markers[17].position.y == -25);
    for (const auto& marker : PathDebug::markers(snapshot, 51))
    {
        CHECK(marker.model == 51);
    }
    snapshot.cursor = snapshot.points.size();
    CHECK(PathDebug::markers(snapshot).empty());
    CHECK(PathDebug::markers({}).empty());
}

TEST_CASE("client IDs are bounded, atomic, quarantined, and allocator-local", "[pathdebug]")
{
    Scheduler     scheduler(1);
    CZoneEntities first(scheduler, {}, nullptr);
    CZoneEntities second(scheduler, {}, nullptr);
    const auto    ids = first.ReserveClientEntityIds(0x1FF);
    REQUIRE(ids.size() == 0x1FF);
    CHECK(ids.front() == 0x700);
    CHECK(ids.back() == 0x8FE);
    CHECK(first.ReserveClientEntityIds(2).empty());
    CHECK(first.GetUsedDynamicTargIDsCount() == 0x1FF);
    const auto last = first.ReserveClientEntityIds(1);
    REQUIRE(last.size() == 1);
    CHECK(last.front() == 0x8FF);
    CHECK(first.ReserveClientEntityIds(1).empty());
    CHECK(second.ReserveClientEntityIds(1).front() == 0x700);
    CHECK(second.GetUsedDynamicTargIDsCount() == 1);
    CHECK(first.GetEntity(ids.front()) == nullptr); // requests/actions cannot resolve a server entity
    first.ReleaseClientEntityId(ids.front());
    first.ReleaseClientEntityId(ids.front()); // repeated release cannot extend or duplicate retirement
    CHECK(first.ReserveClientEntityIds(1).empty());
    CHECK(first.IsClientEntityId(ids.front())); // late info requests are still recognized
    const auto oldOffset = timer::time_offset;
    timer::add_offset(61s);
    first.EraseStaleDynamicTargIDs();
    timer::time_offset = oldOffset;
    CHECK_FALSE(first.IsClientEntityId(ids.front()));
    const auto reused = first.ReserveClientEntityIds(1);
    REQUIRE(reused.size() == 1);
    CHECK(reused.front() == ids.front());
    CHECK(first.GetUsedDynamicTargIDsCount() == 0x200);
}

TEST_CASE("client marker queue replaces stale movement and spawns before despawn", "[pathdebug]")
{
    EmptyDatabase database;
    CCharEntity   observer;
    observer.health.hp = 1;
    PathDebugMarker marker{ { 1, -5, 3, 0, 0 }, 51, "NEXT 1" };
    observer.queueClientEntityPacket(100, std::make_unique<CPathDebugPacket>(100, 0x700, &marker));
    marker.position.x = 10;
    observer.queueClientEntityPacket(100, std::make_unique<CPathDebugPacket>(100, 0x700, &marker));
    auto pending = observer.getPacketListCopy();
    REQUIRE(pending.size() == 1);
    CHECK(pending.front()->ref<float>(0x0C) == 10);
    observer.queueClientEntityPacket(100, std::make_unique<CPathDebugPacket>(100, 0x700, nullptr));
    pending = observer.getPacketListCopy();
    REQUIRE(pending.size() == 1);
    CHECK(observer.popPacket()->ref<uint8>(0x0A) == 0x30);
    observer.queueClientEntityPacket(100, std::make_unique<CPathDebugPacket>(100, 0x700, &marker));
    observer.queueClientEntityPacket(100, nullptr);
    CHECK(observer.getPacketListCopy().empty());
}
