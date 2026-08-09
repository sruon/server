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

#include "detour_navmesh.h"

#include <DetourCommon.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>

#include "common/utils.h"
#include "common/xirand.h"

#include <algorithm>
#include <common/types/maybe.h>
#include <fstream>
#include <vector>

namespace
{

constexpr int kNavmeshSetMagic   = 'M' << 24 | 'S' << 16 | 'E' << 8 | 'T'; // 'MSET'
constexpr int kNavmeshSetVersion = 1;

// Mirrors SamplePolyFlags from RecastDemo's Sample.h.
enum SamplePolyFlags : uint16_t
{
    SAMPLE_POLYFLAGS_WALK     = 0x0001, // Walk (ground, grass, road)
    SAMPLE_POLYFLAGS_SWIM     = 0x0002, // Swim (water)
    SAMPLE_POLYFLAGS_DOOR     = 0x0004, // Move through doors
    SAMPLE_POLYFLAGS_JUMP     = 0x0008, // Jump
    SAMPLE_POLYFLAGS_DISABLED = 0x0010, // Disabled polygon
    SAMPLE_POLYFLAGS_ALL      = 0xFFFF, // All abilities
};

// Exclude wins over include, so include all and exclude the unwanted.
constexpr unsigned short kIncludeFlags = SamplePolyFlags::SAMPLE_POLYFLAGS_ALL;
constexpr unsigned short kExcludeFlags = SamplePolyFlags::SAMPLE_POLYFLAGS_DISABLED;

// Detour result buffer sizes; queries run blocking, so keep these as small as quality allows.

// From docs: Maximum number of search nodes.
constexpr size_t kMaxNavPolys = 2048;

// TODO: should this scale with the number of tiles in a navmesh, up to the nearest power of two?
constexpr size_t kPathPolyLimit = 512;

// From docs: The maximum number of polygons the visited array can hold.
constexpr size_t kMaxQueryPolys = 16;

// Detour search extents, used to snap a query point onto the nearest poly.
constexpr float smallPolyPickExt[3] = { 0.5f, 1.0f, 0.5f };
constexpr float polyPickExt[3]      = { 2.5f, 5.0f, 2.5f };
constexpr float largePolyPickExt[3] = { 30.0f, 60.0f, 30.0f };

struct NavMeshSetHeader
{
    int             magic;
    int             version;
    int             numTiles;
    dtNavMeshParams params;
};

struct NavMeshTileHeader
{
    dtTileRef tileRef;
    int       dataSize;
};

// Drop interior points within `eps` of the line between their kept neighbours (packed xyz triples).
void dropCollinearPoints(std::vector<float>& points, float eps)
{
    if (points.size() < 9)
    {
        return;
    }

    std::vector<float> kept;
    kept.reserve(points.size());
    kept.push_back(points[0]);
    kept.push_back(points[1]);
    kept.push_back(points[2]);

    for (size_t i = 3; i + 3 < points.size(); i += 3)
    {
        const size_t last  = kept.size() - 3;
        const float  ax    = kept[last + 0];
        const float  az    = kept[last + 2];
        const float  bx    = points[i + 3];
        const float  bz    = points[i + 5];
        const float  px    = points[i + 0];
        const float  pz    = points[i + 2];
        const float  dx    = bx - ax;
        const float  dz    = bz - az;
        const float  lenSq = dx * dx + dz * dz;

        float deviation = eps + 1.0f;
        if (lenSq > 1e-6f)
        {
            const float t     = ((px - ax) * dx + (pz - az) * dz) / lenSq;
            const float projX = ax + t * dx;
            const float projZ = az + t * dz;
            deviation         = std::sqrt((px - projX) * (px - projX) + (pz - projZ) * (pz - projZ));
        }

        if (deviation > eps)
        {
            kept.push_back(points[i + 0]);
            kept.push_back(points[i + 1]);
            kept.push_back(points[i + 2]);
        }
    }

    const size_t n = points.size();
    kept.push_back(points[n - 3]);
    kept.push_back(points[n - 2]);
    kept.push_back(points[n - 1]);
    points.swap(kept);
}

} // namespace

// Detour is right-handed Y-up, FFXI is left-handed Y-up: Y and Z are sign-flipped between them.
auto DetourNavMesh::toDetour(const position_t& p) -> std::array<float, 3>
{
    return { p.x, p.y * -1.0f, p.z * -1.0f };
}

auto DetourNavMesh::fromDetour(const float* p) -> position_t
{
    return { p[0], p[1] * -1.0f, p[2] * -1.0f, 0, 0 };
}

auto DetourNavMesh::makeFilter() const -> dtQueryFilter
{
    dtQueryFilter filter;
    filter.setIncludeFlags(kIncludeFlags);
    filter.setExcludeFlags(kExcludeFlags);
    return filter;
}

auto DetourNavMesh::lookupPoly(const std::array<float, 3>& pos, const float* extents, const dtQueryFilter& filter) const -> Maybe<DetourNavMesh::PolyLookup>
{
    PolyLookup hit{};
    const auto status = navMeshQuery_.findNearestPoly(pos.data(), extents, &filter, &hit.ref, hit.nearest.data());
    if (dtStatusFailed(status))
    {
        ShowError("DetourNavMesh::lookupPoly findNearestPoly failed (%u): %s", zoneID_, detourStatusString(status));
        return std::nullopt;
    }
    if (!navMesh_->isValidPolyRef(hit.ref))
    {
        return std::nullopt;
    }
    return hit;
}

DetourNavMesh::DetourNavMesh(uint16 zoneID)
: zoneID_(zoneID)
, navMesh_(nullptr)
{
    navMeshQueryPolyData_.resize(kMaxNavPolys);
    navMeshQueryStraightPathFloatData_.resize(kMaxNavPolys * 3);
    navMeshQueryStraightPathFlagData_.resize(kMaxNavPolys);
    navMeshQueryStraightPathPolyData_.resize(kMaxNavPolys);
}

DetourNavMesh::~DetourNavMesh()
{
    if (navMesh_)
    {
        dtFreeNavMesh(navMesh_);
    }
}

auto DetourNavMesh::load(const std::string& filename) -> bool
{
    this->filename_ = filename;

    std::ifstream file(filename.c_str(), std::ios_base::in | std::ios_base::binary);

    if (!file.good())
    {
        return false;
    }

    // Read header
    NavMeshSetHeader header{};
    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (header.magic != kNavmeshSetMagic)
    {
        return false;
    }

    if (header.version != kNavmeshSetVersion)
    {
        return false;
    }

    navMesh_ = dtAllocNavMesh();
    if (!navMesh_)
    {
        return false;
    }

    dtStatus status = navMesh_->init(&header.params);
    if (dtStatusFailed(status))
    {
        ShowError("DetourNavMesh::load Could not initialize detour for (%s)", filename);
        ShowError(detourStatusString(status));
        return false;
    }

    // Read tiles
    for (int i = 0; i < header.numTiles; ++i)
    {
        NavMeshTileHeader tileHeader{};
        file.read(reinterpret_cast<char*>(&tileHeader), sizeof(tileHeader));
        if (!tileHeader.tileRef || !tileHeader.dataSize)
        {
            break;
        }

        unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
        if (!data)
        {
            break;
        }
        std::memset(data, 0, tileHeader.dataSize);
        file.read(reinterpret_cast<char*>(data), tileHeader.dataSize);

        navMesh_->addTile(data, tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef, nullptr);
    }

    // Init the path finder query
    status = navMeshQuery_.init(navMesh_, kMaxNavPolys);

    if (dtStatusFailed(status))
    {
        ShowError("DetourNavMesh::load Error loading navMeshQuery_ (%s)", filename);
        ShowError(detourStatusString(status));
        return false;
    }

    return true;
}

auto DetourNavMesh::unload() -> void
{
    dtFreeNavMesh(navMesh_);
    navMesh_ = nullptr;
}

auto DetourNavMesh::installNavMesh(dtNavMesh* newNavMesh) -> bool
{
    if (!newNavMesh)
    {
        return false;
    }

    unload();

    navMesh_ = newNavMesh;

    const auto status = navMeshQuery_.init(navMesh_, kMaxNavPolys);
    if (dtStatusFailed(status))
    {
        ShowErrorFmt("DetourNavMesh::installNavMesh: Could not init navMeshQuery ({})", zoneID_);
        unload();
        return false;
    }

    return true;
}

auto DetourNavMesh::save(const std::string& path) const -> bool
{
    if (!navMesh_ || path.empty())
    {
        return false;
    }

    std::ofstream file(path, std::ios::binary);
    if (!file.good())
    {
        ShowErrorFmt("DetourNavMesh::save: Could not open file for writing ({})", path);
        return false;
    }

    const auto* nav = navMesh_;

    auto header    = NavMeshSetHeader{};
    header.magic   = kNavmeshSetMagic;
    header.version = kNavmeshSetVersion;
    header.params  = *nav->getParams();

    for (auto i = 0; i < nav->getMaxTiles(); ++i)
    {
        const auto* tile = nav->getTile(i);
        if (tile && tile->header && tile->dataSize > 0)
        {
            header.numTiles++;
        }
    }

    file.write(reinterpret_cast<const char*>(&header), sizeof(header));

    for (auto i = 0; i < nav->getMaxTiles(); ++i)
    {
        const auto* tile = nav->getTile(i);
        if (!tile || !tile->header || tile->dataSize <= 0)
        {
            continue;
        }

        const auto tileHeader = NavMeshTileHeader{
            .tileRef  = nav->getTileRef(tile),
            .dataSize = tile->dataSize,
        };

        file.write(reinterpret_cast<const char*>(&tileHeader), sizeof(tileHeader));
        file.write(reinterpret_cast<const char*>(tile->data), tile->dataSize);
    }

    return true;
}

auto DetourNavMesh::findPath(const position_t& start, const position_t& end, float clearance) -> Maybe<PathResult>
{
    TracyZoneScoped;

    if (std::isnan(start.x) || std::isnan(start.y) || std::isnan(start.z) ||
        std::isnan(end.x) || std::isnan(end.y) || std::isnan(end.z))
    {
        ShowWarning("DetourNavMesh::findPath NaN position detected (%u)", zoneID_);
        return std::nullopt;
    }

    DebugNavmesh("DetourNavMesh::findPath (%f, %f, %f) -> (%f, %f, %f) (zone: %u) (kMaxNavPolys: %u)",
                 start.x,
                 start.y,
                 start.z,
                 end.x,
                 end.y,
                 end.z,
                 zoneID_,
                 kMaxNavPolys);

    const auto filter    = makeFilter();
    const auto startPoly = lookupPoly(toDetour(start), polyPickExt, filter);
    if (!startPoly)
    {
        DebugNavmesh("DetourNavMesh::findPath start has no poly within extents: (%f, %f, %f) (%u)", start.x, start.y, start.z, zoneID_);
        return std::nullopt;
    }

    const auto endPoly = lookupPoly(toDetour(end), polyPickExt, filter);
    if (!endPoly)
    {
        DebugNavmesh("DetourNavMesh::findPath end has no poly within extents: (%f, %f, %f) (%u)", end.x, end.y, end.z, zoneID_);
        return std::nullopt;
    }

    // Build a poly corridor from start to end
    int32    pathPolyCount = 0;
    dtStatus status        = navMeshQuery_.findPath(
        startPoly->ref, endPoly->ref, startPoly->nearest.data(), endPoly->nearest.data(), &filter, navMeshQueryPolyData_.data(), &pathPolyCount, static_cast<int>(kPathPolyLimit));
    if (dtStatusFailed(status))
    {
        ShowError("DetourNavMesh::findPath findPath error (%u)", zoneID_);
        ShowError(detourStatusString(status));
        return std::nullopt;
    }

    const bool detourPartial = (status & DT_PARTIAL_RESULT) != 0;

    if (pathPolyCount <= 0)
    {
        ShowError("DetourNavMesh::findPath Unable to generate polys for path (%f, %f, %f)->(%f, %f, %f) (%u)",
                  start.x,
                  start.y,
                  start.z,
                  end.x,
                  end.y,
                  end.z,
                  zoneID_);
        return std::nullopt;
    }

    // Straighten the corridor into waypoints; DT_STRAIGHTPATH_ALL_CROSSINGS worsens local-minima trapping.
    int32 straightPathCount = 0;
    status                  = navMeshQuery_.findStraightPath(
        startPoly->nearest.data(), endPoly->nearest.data(), navMeshQueryPolyData_.data(), pathPolyCount, navMeshQueryStraightPathFloatData_.data(), navMeshQueryStraightPathFlagData_.data(), navMeshQueryStraightPathPolyData_.data(), &straightPathCount, static_cast<int>(kPathPolyLimit) /* , DT_STRAIGHTPATH_ALL_CROSSINGS */);
    if (dtStatusFailed(status))
    {
        ShowError("DetourNavMesh::findPath findStraightPath error (%u)", zoneID_);
        ShowError(detourStatusString(status));
        return std::nullopt;
    }

    // Drop the best-guess final waypoint of a partial path, since it lands far from the request and traps the entity.
    const float* pathEnd = &navMeshQueryStraightPathFloatData_[(straightPathCount - 1) * 3];
    const float* wantEnd = endPoly->nearest.data();
    const float  dx      = pathEnd[0] - wantEnd[0];
    const float  dy      = pathEnd[1] - wantEnd[1];
    const float  dz      = pathEnd[2] - wantEnd[2];

    // A straight corridor yields two points however far short it stopped, so the count says nothing about arrival.
    bool isPartial = false;
    if (detourPartial && dx * dx + dy * dy + dz * dz > 5.0f * 5.0f)
    {
        DebugNavmesh("DetourNavMesh::findPath Partial path detected! (%u)", zoneID_);
        isPartial = true;

        // Trimming a two-point path would leave only the start.
        if (straightPathCount > 2)
        {
            straightPathCount -= 1;
        }
    }

    // straightPathCount == 1: only the start position was produced - no usable path
    if (straightPathCount <= 1)
    {
        return std::nullopt;
    }

    // TODO: Detect local minima and re-try pathing with a larger buffer.

    // Radius-0 mesh: findStraightPath leaves waypoints hard against walls; widen the corridor for the body.
    straightPathCount = insetPathForBody(straightPathCount, clearance, filter);

    // Skip index 0 (the start position - we're already there)
    std::vector<pathpoint_t> outPoints;
    outPoints.reserve(straightPathCount - 1);
    for (int i = 1; i < straightPathCount; ++i)
    {
        outPoints.emplace_back(pathpoint_t{
            fromDetour(&navMeshQueryStraightPathFloatData_[i * 3]),
            0s,
            false,
        });
    }

    return PathResult{ std::move(outPoints), isPartial };
}

auto DetourNavMesh::insetPathForBody(int pointCount, float clearance, const dtQueryFilter& filter) -> int
{
    // Point-sized agent, or nothing worth insetting.
    if (!(clearance > 0.0f && pointCount >= 2 && navMeshQueryStraightPathPolyData_[0]))
    {
        return pointCount;
    }

    // Inset the center this far so the body edge (a hitbox radius in) clears too; ~retail's 1.5-2y corner berth.
    constexpr float kBerthMargin = 1.5f;
    const float     berth        = clearance + kBerthMargin;

    std::vector<float> widened = densifyOffWalls(pointCount, berth, filter);
    relaxTowardClearance(widened, berth, filter);

    // Drop collinear interior points; only the ones that bow off a wall matter. Eps keeps only near-straight runs.
    dropCollinearPoints(widened, 0.1f);

    pointCount = static_cast<int>(widened.size() / 3);
    for (int i = 0; i < pointCount * 3; ++i)
    {
        navMeshQueryStraightPathFloatData_[i] = widened[i];
    }

    return pointCount;
}

auto DetourNavMesh::densifyOffWalls(int pointCount, float berth, const dtQueryFilter& filter) -> std::vector<float>
{
    std::vector<float> widened;
    widened.reserve(static_cast<size_t>(pointCount) * 3 * 4);
    widened.push_back(navMeshQueryStraightPathFloatData_[0]);
    widened.push_back(navMeshQueryStraightPathFloatData_[1]);
    widened.push_back(navMeshQueryStraightPathFloatData_[2]);

    float lastEmitted[3];
    dtVcopy(lastEmitted, &navMeshQueryStraightPathFloatData_[0]);
    dtPolyRef polyRef = navMeshQueryStraightPathPolyData_[0];

    for (int i = 0; i + 1 < pointCount && widened.size() / 3 + 2 < kMaxNavPolys; ++i)
    {
        const float* from = &navMeshQueryStraightPathFloatData_[i * 3];
        const float* to   = &navMeshQueryStraightPathFloatData_[(i + 1) * 3];

        float segment[3];
        dtVsub(segment, to, from);
        segment[1]          = 0.0f;
        const float segLen  = dtVlen(segment);
        const int   steps   = std::clamp(static_cast<int>(segLen / (berth * 0.5f)) + 1, 2, 48);
        const bool  lastSeg = (i + 2 == pointCount);

        for (int step = 1; step <= steps && widened.size() / 3 + 1 < kMaxNavPolys; ++step)
        {
            const bool isEndpoint  = (step == steps);
            const bool finalTarget = isEndpoint && lastSeg;

            float sample[3];
            if (isEndpoint)
            {
                dtVcopy(sample, to);
            }
            else
            {
                dtVlerp(sample, from, to, static_cast<float>(step) / static_cast<float>(steps));
            }

            dtPolyRef visited[kMaxQueryPolys];
            int       visitedCount = 0;
            float     onMesh[3];
            if (dtStatusFailed(navMeshQuery_.moveAlongSurface(polyRef, lastEmitted, sample, &filter, onMesh, visited, &visitedCount, kMaxQueryPolys)))
            {
                continue;
            }
            if (visitedCount > 0)
            {
                polyRef = visited[visitedCount - 1];
            }

            // finalTarget is the mob's actual target, emitted as-is; every other sample is pushed off walls.
            if (!finalTarget && isEndpoint)
            {
                // Corner: push out along the leg bisector; findDistanceToWall's normal is degenerate on a vertex.
                const float* next = &navMeshQueryStraightPathFloatData_[(i + 2) * 3];
                float        legPrev[3];
                float        legNext[3];
                dtVsub(legPrev, from, onMesh);
                dtVsub(legNext, next, onMesh);
                legPrev[1] = 0.0f;
                legNext[1] = 0.0f;
                if (dtVlen(legPrev) > 1e-4f && dtVlen(legNext) > 1e-4f)
                {
                    dtVnormalize(legPrev);
                    dtVnormalize(legNext);
                    float bisector[3];
                    dtVadd(bisector, legPrev, legNext);
                    if (dtVlen(bisector) >= 0.1f)
                    {
                        dtVnormalize(bisector);
                        float target[3];
                        dtVmad(target, onMesh, bisector, -berth);
                        float pushed[3];
                        if (dtStatusSucceed(navMeshQuery_.moveAlongSurface(polyRef, onMesh, target, &filter, pushed, visited, &visitedCount, kMaxQueryPolys)))
                        {
                            const float dy = pushed[1] - onMesh[1];
                            if (dy > -berth && dy < berth)
                            {
                                dtVcopy(onMesh, pushed);
                                if (visitedCount > 0)
                                {
                                    polyRef = visited[visitedCount - 1];
                                }
                            }
                        }
                    }
                }
            }
            else if (!finalTarget)
            {
                float wallDist   = 0.0f;
                float hitPos[3]  = {};
                float hitNorm[3] = {};
                if (dtStatusSucceed(navMeshQuery_.findDistanceToWall(polyRef, onMesh, berth, &filter, &wallDist, hitPos, hitNorm)) && wallDist < berth)
                {
                    float target[3];
                    dtVmad(target, onMesh, hitNorm, berth - wallDist);
                    float pushed[3];
                    if (dtStatusSucceed(navMeshQuery_.moveAlongSurface(polyRef, onMesh, target, &filter, pushed, visited, &visitedCount, kMaxQueryPolys)))
                    {
                        const float dy = pushed[1] - onMesh[1];
                        if (dy > -berth && dy < berth)
                        {
                            dtVcopy(onMesh, pushed);
                            if (visitedCount > 0)
                            {
                                polyRef = visited[visitedCount - 1];
                            }
                        }
                    }
                }
            }

            widened.push_back(onMesh[0]);
            widened.push_back(onMesh[1]);
            widened.push_back(onMesh[2]);
            dtVcopy(lastEmitted, onMesh);
        }
    }

    return widened;
}

auto DetourNavMesh::relaxTowardClearance(std::vector<float>& widened, float berth, const dtQueryFilter& filter) -> void
{
    // Lift any point still against a wall toward clearance; 8-way sampling handles the degenerate on-wall normal and bows corners away instead of cutting them.
    static const float kAscentDirs[8][2] = { { 1.0f, 0.0f }, { 0.70711f, 0.70711f }, { 0.0f, 1.0f }, { -0.70711f, 0.70711f }, { -1.0f, 0.0f }, { -0.70711f, -0.70711f }, { 0.0f, -1.0f }, { 0.70711f, -0.70711f } };
    constexpr float    kAscentStep       = 0.6f;
    for (int iter = 0; iter < 4 && widened.size() >= 9; ++iter)
    {
        for (size_t i = 3; i + 3 < widened.size(); i += 3)
        {
            const std::array<float, 3> cur  = { widened[i], widened[i + 1], widened[i + 2] };
            const auto                 poly = lookupPoly(cur, polyPickExt, filter);
            if (!poly)
            {
                continue;
            }

            float hitPos[3]  = {};
            float hitNorm[3] = {};
            float bestClear  = 0.0f;
            if (dtStatusFailed(navMeshQuery_.findDistanceToWall(poly->ref, cur.data(), berth, &filter, &bestClear, hitPos, hitNorm)) || bestClear >= berth)
            {
                continue; // off-mesh or already clear enough
            }

            float best[3];
            dtVcopy(best, cur.data());
            for (int d = 0; d < 8; ++d)
            {
                const float target[3] = { cur[0] + kAscentDirs[d][0] * kAscentStep, cur[1], cur[2] + kAscentDirs[d][1] * kAscentStep };
                dtPolyRef   visited[kMaxQueryPolys];
                int         visitedCount = 0;
                float       cand[3];
                if (dtStatusFailed(navMeshQuery_.moveAlongSurface(poly->ref, cur.data(), target, &filter, cand, visited, &visitedCount, kMaxQueryPolys)))
                {
                    continue;
                }
                const dtPolyRef candRef = visitedCount > 0 ? visited[visitedCount - 1] : poly->ref;
                float           clear   = 0.0f;
                if (dtStatusSucceed(navMeshQuery_.findDistanceToWall(candRef, cand, berth, &filter, &clear, hitPos, hitNorm)) && clear > bestClear)
                {
                    bestClear = clear;
                    dtVcopy(best, cand);
                }
            }

            widened[i]     = best[0];
            widened[i + 1] = best[1];
            widened[i + 2] = best[2];
        }
    }
}

auto DetourNavMesh::findRandomPosition(const position_t& start, float maxRadius) const -> Maybe<position_t>
{
    TracyZoneScoped;

    DebugNavmesh("DetourNavMesh::findRandomPosition (%f, %f, %f) (%u)", start.x, start.y, start.z, zoneID_);

    const auto filter    = makeFilter();
    const auto spos      = toDetour(start);
    const auto startPoly = lookupPoly(spos, polyPickExt, filter);
    if (!startPoly)
    {
        DebugNavmesh("DetourNavMesh::findRandomPosition start has no poly within extents: (%f, %f, %f) (%u)", start.x, start.y, start.z, zoneID_);
        return std::nullopt;
    }

    dtPolyRef  randomRef = 0;
    float      randomPt[3];
    const auto status = navMeshQuery_.findRandomPointAroundCircle(
        startPoly->ref, spos.data(), maxRadius, &filter, []() -> float
        {
            return xirand::GetRandomNumber(1.0f);
        },
        &randomRef,
        randomPt);

    if (dtStatusFailed(status))
    {
        ShowError("DetourNavMesh::findRandomPosition error (%u): %s", zoneID_, detourStatusString(status));
        return std::nullopt;
    }

    return fromDetour(randomPt);
}

auto DetourNavMesh::validPosition(const position_t& position) const -> bool
{
    TracyZoneScoped;

    DebugNavmesh("DetourNavMesh::validPosition (%f, %f, %f) (%u)", position.x, position.y, position.z, zoneID_);
    return lookupPoly(toDetour(position), smallPolyPickExt, makeFilter()).has_value();
}

auto DetourNavMesh::findClosestValidPoint(const position_t& position) const -> Maybe<position_t>
{
    TracyZoneScoped;

    DebugNavmesh("DetourNavMesh::findClosestValidPoint (%f, %f, %f) (%u)", position.x, position.y, position.z, zoneID_);

    const auto hit = lookupPoly(toDetour(position), largePolyPickExt, makeFilter());
    if (!hit)
    {
        return std::nullopt;
    }

    return fromDetour(hit->nearest.data());
}

auto DetourNavMesh::findFurthestValidPoint(const position_t& startPosition, const position_t& endPosition) const -> Maybe<position_t>
{
    TracyZoneScoped;

    DebugNavmesh("DetourNavMesh::findFurthestValidPoint (%f, %f, %f) -> (%f, %f, %f) (%u)",
                 startPosition.x,
                 startPosition.y,
                 startPosition.z,
                 endPosition.x,
                 endPosition.y,
                 endPosition.z,
                 zoneID_);

    const auto filter    = makeFilter();
    const auto startPoly = lookupPoly(toDetour(startPosition), largePolyPickExt, filter);
    if (!startPoly)
    {
        return std::nullopt;
    }

    const auto targetPos = toDetour(endPosition);
    dtPolyRef  visited[kMaxQueryPolys];
    int        visitedCount = 0;
    float      result[3];

    const auto status = navMeshQuery_.moveAlongSurface(
        startPoly->ref, startPoly->nearest.data(), targetPos.data(), &filter, result, visited, &visitedCount, kMaxQueryPolys);

    if (dtStatusFailed(status))
    {
        return std::nullopt;
    }

    return fromDetour(result);
}

auto DetourNavMesh::moveAlongSurface(const position_t& start, const position_t& end, position_t& result) const -> bool
{
    TracyZoneScoped;

    // A tight start extent assumes the entity is on the mesh; report failure otherwise so the caller free-moves back on.
    const auto filter    = makeFilter();
    const auto startPoly = lookupPoly(toDetour(start), polyPickExt, filter);
    if (!startPoly)
    {
        return false;
    }

    const auto targetPos = toDetour(end);
    dtPolyRef  visited[kMaxQueryPolys];
    int        visitedCount = 0;
    float      out[3];

    const auto status = navMeshQuery_.moveAlongSurface(
        startPoly->ref, startPoly->nearest.data(), targetPos.data(), &filter, out, visited, &visitedCount, kMaxQueryPolys);

    if (dtStatusFailed(status))
    {
        return false;
    }

    result = fromDetour(out);
    return true;
}

auto DetourNavMesh::snapToValidPosition(position_t& position) const -> void
{
    TracyZoneScoped;

    DebugNavmesh("DetourNavMesh::snapToValidPosition (%f, %f, %f) (%u)", position.x, position.y, position.z, zoneID_);

    const auto filter = makeFilter();
    const auto spos   = toDetour(position);

    // Try a cheap small radius first, then 30f XZ for genuinely off-mesh positions.
    auto poly = lookupPoly(spos, polyPickExt, filter);
    if (!poly)
    {
        poly = lookupPoly(spos, largePolyPickExt, filter);
    }
    if (!poly)
    {
        return;
    }

    const auto snapped = fromDetour(poly->nearest.data());
    position.x         = snapped.x;
    position.y         = snapped.y;
    position.z         = snapped.z;
}

[[nodiscard]] auto DetourNavMesh::detourStatusString(const uint32 status) -> std::string
{
    std::string outStr;

    // High-level status
    if (status & DT_FAILURE)
    {
        outStr += "DT_FAILURE: Operation failed. ";
    }
    if (status & DT_SUCCESS)
    {
        outStr += "DT_SUCCESS: Operation succeeded. ";
    }
    if (status & DT_IN_PROGRESS)
    {
        outStr += "DT_IN_PROGRESS: Operation still in progress. ";
    }

    // Status detail bits
    if (status & DT_WRONG_MAGIC)
    {
        outStr += "DT_WRONG_MAGIC: Input data is not recognized. ";
    }
    if (status & DT_WRONG_VERSION)
    {
        outStr += "DT_WRONG_VERSION: Input data is in wrong version. ";
    }
    if (status & DT_OUT_OF_MEMORY)
    {
        outStr += "DT_OUT_OF_MEMORY: Operation ran out of memory. ";
    }
    if (status & DT_INVALID_PARAM)
    {
        outStr += "DT_INVALID_PARAM: An input parameter was invalid. ";
    }
    if (status & DT_BUFFER_TOO_SMALL)
    {
        outStr += "DT_BUFFER_TOO_SMALL: Result buffer for the query was too small to store all results. ";
    }
    if (status & DT_OUT_OF_NODES)
    {
        outStr += "DT_OUT_OF_NODES: Query ran out of nodes during search. ";
    }
    if (status & DT_PARTIAL_RESULT)
    {
        outStr += "DT_PARTIAL_RESULT: Query did not reach the end location, returning best guess. ";
    }
    if (status & DT_ALREADY_OCCUPIED)
    {
        outStr += "DT_ALREADY_OCCUPIED: A tile has already been assigned to the given x, y coordinate. ";
    }

    return outStr;
}
