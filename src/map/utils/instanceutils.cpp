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

#include "instanceutils.h"

#include "common/logging.h"
#include "data/loader.h"

#include <common/types/hash_map.h>

#include "lua/luautils.h"

#include "entities/char_entity.h"
#include "instance_loader.h"
#include "zoneutils.h"

#include <coroutine>
#include <filesystem>
#include <queue>

#include <fmt/ranges.h>

namespace instanceutils
{

namespace
{

HashMap<uint16, InstanceData_t>       InstanceCatalog;
HashMap<uint16, InstanceData_t>       InstanceData;
std::queue<std::pair<uint32, uint16>> LoadQueue; // player id, instance id
detail::LazyLoadState                 lazyLoad;

auto ReadInstancesAssignedToThisProcess(IPP mapIPP) -> std::vector<uint16>
{
    std::vector<uint16> result;

    const auto rset = db::preparedStmt("SELECT zoneid, name FROM zone_settings "
                                       "WHERE IF(? <> 0, ? = zoneip AND ? = zoneport, TRUE)",
                                       mapIPP.getIP(),
                                       mapIPP.getIPString(),
                                       mapIPP.getPort());
    FOR_DB_MULTIPLE_RESULTS(rset)
    {
        const auto zoneId   = rset->get<xi::ZoneId>("zoneid");
        const auto zoneName = rset->get<std::string>("name");

        for (const auto& instance : xi::data::loadZoneFile<xi::data::datasets::zones::instances::Dataset>(zoneId).value_or(xi::data::Instances{}))
        {
            InstanceCatalog[instance.Id] = InstanceData_t{
                .id                 = instance.Id,
                .instance_name      = instance.Name,
                .instance_zone      = zoneId,
                .instance_zone_name = zoneName,
                .entrance_zone      = static_cast<uint16>(instance.Entrance),
                .overlay            = instance.Overlay,
                .time_limit         = instance.TimeLimit,
                .entry              = instance.Entry,
                .music              = instance.Music,
            };

            result.emplace_back(instance.Id);
        }
    }

    return result;
}

auto LoadInstances(const std::vector<uint16>& instanceIds) -> void
{
    for (const auto instanceId : instanceIds)
    {
        const auto entry = InstanceCatalog.find(instanceId);
        if (entry == InstanceCatalog.end())
        {
            continue;
        }

        auto data = entry->second;

        // Determine if instance exists at new assault path
        data.filename = fmt::format("./scripts/assaults/{}/{}.lua", data.instance_zone_name, data.instance_name);
        if (std::filesystem::exists(data.filename))
        {
            luautils::LoadLuaObjectFromFile(data.filename, true);
        }

        // If not, fall back to regular instance path
        else
        {
            data.filename = fmt::format("./scripts/zones/{}/instances/{}.lua", data.instance_zone_name, data.instance_name);
            luautils::LoadLuaObjectFromFile(data.filename);
        }

        // Add to data cache
        InstanceData[data.id] = data;
    }
}

} // namespace

// Initialize instance loading: immediate (load all now) or lazy (load on first access)
auto Initialize(MapConfig config) -> void
{
    const auto instanceIds = ReadInstancesAssignedToThisProcess(config.ipp);

    if (!config.lazyZones)
    {
        LoadInstances(instanceIds);
        return;
    }

    lazyLoad.enabled          = true;
    lazyLoad.managedInstances = std::set(instanceIds.begin(), instanceIds.end());
}

// NOTE: This used to be multithreaded, but was starting to cause problems with repeated loading
//       and loading in quick succession, so we've swapped it out for a queue which services a
//       single request at the end of every tick.
// TODO: Make this multithreaded and not blocking the main tick loop
auto CheckInstance(Scheduler& scheduler, MapConfig config) -> Task<void>
{
    if (LoadQueue.empty())
    {
        co_return;
    }

    const auto requestPair = LoadQueue.front();
    auto*      PRequester  = zoneutils::GetChar(requestPair.first);
    if (!PRequester)
    {
        ShowError("Encountered invalid requester id when loading instance!");
        LoadQueue.pop();
        co_return;
    }

    const auto instanceId = requestPair.second;
    const auto data       = GetInstanceData(instanceId);

    // CInstanceLoader requires the instance template zone to be loaded.
    const bool zoneReady = co_await zoneutils::IsZoneReady(scheduler, config, data.instance_zone);
    if (!zoneReady)
    {
        co_return;
    }

    LoadQueue.pop();

    auto loader = std::make_unique<CInstanceLoader>(instanceId, PRequester);
    loader->LoadInstance();

    co_return;
}

auto LoadInstance(uint32 instanceid, CCharEntity* PRequester) -> void
{
    LoadQueue.emplace(PRequester->id, instanceid);
}

auto GetInstanceData(uint32 instanceid) -> InstanceData_t
{
    const auto key = static_cast<uint16>(instanceid);

    if (auto it = InstanceData.find(key); it != InstanceData.end())
    {
        return it->second;
    }

    if (lazyLoad.enabled && lazyLoad.managedInstances.contains(key))
    {
        LoadInstances({ key });

        if (auto it = InstanceData.find(key); it != InstanceData.end())
        {
            return it->second;
        }

        ShowWarning("instanceutils::GetInstanceData: managed instanceid %u is in no instances.yaml", instanceid);
    }

    return {};
}

auto IsValidInstanceID(uint32 instanceid) -> bool
{
    const auto key = static_cast<uint16>(instanceid);

    if (InstanceData.contains(key))
    {
        return true;
    }

    return lazyLoad.enabled && lazyLoad.managedInstances.contains(key);
}

}; // namespace instanceutils
