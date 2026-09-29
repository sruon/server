# Mob path markers

GM permission level 1 is required. Target a living mob, then use:

```text
!pathdebug               Watch its active path, refreshing at most every 250 ms
!pathdebug freeze        Hold the current route, or wait for the next if idle
!pathdebug on            Resume updating the markers as the mob moves
!pathdebug off           Despawn your markers and release the reservation
!pathdebug on 51         Watch with Homepoint Crystal model 51 for visibility testing
!pathdebug freeze 2420   Capture using Waypoint model 2420
```

## Arbitrary points

`player:pathDebugPoints(points, model)` loads any number of caller-supplied
points with no mob attached. Points within 50 yalms of the observer are shown,
refreshed every 250 ms as they move. A point set reserves every free dynamic ID
it could use (up to all 512), so pets, trusts and dynamic entities cannot spawn
in that zone until `!pathdebug off` and the 60-second quarantine. Each point is `{ x, y, z }` or `{ x = , y = , z = , name = , model = }`;
the label defaults to its index. `!pathdebug off` clears them, and pointing
`!pathdebug` at a mob replaces them.

`!pathdebug nodes [model]` loads `scripts/zones/<current zone>/pathNodes.lua`,
rereading the file on each use.

The first 16 remaining waypoints are numbered by their position in the active
route. All markers default to Warhorse Hoofprint model 1525. `NEXTn` is the current
cursor, `P` means paused, and `W` means waiting at a scripted waypoint.
`End` marks the current route/chunk endpoint even when it lies beyond those 16
points. `Dest` marks `GetDestination()`, the eventual target of a chunked path.
When endpoint and destination coincide they share `E+D` (or `End+Dest`) to avoid
overlapping labels. Coordinates come directly from the path, with no navmesh
query or ground snapping. The display lifts each marker and its name one unit
above the waypoint (negative Y is up), without changing the actual path.
Marker packets set `Flags0.GroundFlag` (`0x8000` at offset `0x18`) so the client
preserves this height instead of snapping the marker back to the terrain. This
matches the flag used by ships in `src/map/transports/ship.cpp` and the
[NPC packet reference](https://github.com/atom0s/XiPackets/blob/main/world/server/0x000E/README.md#groundflag).

An idle mob clears the live markers and retains the watch. Freeze holds only
the visualization: it does not stop the mob moving. The command reports how many
markers it captured. If the mob has no path yet, it reports that it is waiting,
captures the first nonempty path it observes, then reports the marker count.
The captured route remains displayed while the mob moves or becomes idle.
`!pathdebug on` resumes live updates; `!pathdebug off` also cancels a pending capture.
Both modes stop on target death, despawn, invalid identity, observer zone-out/logout,
or removal of GM permission. Repeating the command or changing its target reuses
the reservation.

Each observer reserves 18 IDs from their zone or instance's existing dynamic
allocator (`0x700`–`0x8FF`). A watch that cannot reserve the entire pool fails
without allocating anything. Released IDs remain quarantined for 60 seconds;
rapid off/on cycling can exhaust the shared pool temporarily. These reservations
consume the same finite capacity as pets, trusts, and dynamic entities.

Markers are lightweight data serialized into private `0x0E` packets. They are
never server NPCs and never enter spawn, targeting, collision, or interaction
lists. The packet disables targeting, interaction and entity collision, and
initializes positions with zero movement time and speed. Updates replace pending
packets for the same reserved ID. Zone-out and reservation expiry remove stale
queued packets. `0x016` and `0x017` requests resend only the requesting observer's
own markers; requests for another observer's or a quarantined ID are consumed.
Target serials prevent a replacement mob with the same IDs from inheriting a watch.

## Validation

Build `xi_map` and `xi_test`. The Catch2 cases tagged `[pathdebug]` run with the
existing C++ tests at `xi_test` startup. They cover serialization, allocation,
quarantine, packet ordering, privacy, request resends, cleanup, instances, ID
reuse, live cursors, waits, patrol restarts and chunk transitions. Test entity and
zone construction uses an empty database backend, never a live game database.

In a game client, check:

1. Watch a roaming mob, then pull it around corners and over slopes. Confirm the
   numbered route changes with its real movement and markers jump directly to
   new positions. Confirm another player sees none of your markers.
2. Exercise a long partial/chunked path. Confirm `End` and `Dest` differ until the
   final chunk, and that the final marker remains visible with a capped route.
3. Check a scripted waypoint wait, paused path, patrol loop and roam turn. Stop
   its path, then restart it; live markers should disappear and return.
4. Freeze a route, move the mob, resume watching, retarget, and repeat `off`.
   Freeze an idle mob and confirm the waiting message, followed by a captured
   marker count when it starts moving. Its markers should then stay in place.
   Kill/despawn the mob and repeat with a dynamic mob and inside an instance.
5. Zone, log out and disconnect while watching. Re-enter the zone and check for
   ghosts. Watch simultaneously with two GMs and spawn pets/trusts nearby.
6. Confirm marker names clear the ground, the display sits one unit above the
   actual path, and markers remain untargetable and have no collision.
   Try model overrides if the hoofprint markers are hard to see.
   If packet tooling is available, request a marker using both information
   packets and verify private resends without entity-not-found warnings.

No game-client rendering or retail packet capture validation was performed for
this feature. The packet layout follows the repository's fixed-model serializer
and the [XiPackets 0x000E reference](https://github.com/atom0s/XiPackets/tree/main/world/server/0x000E).
The hoofprint markers and client handling of position initialization still need the
in-game checks above; server-side tests cannot establish their visual behavior.
