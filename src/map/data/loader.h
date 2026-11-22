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

#include "common/enum_traits.h"
#include "common/logging.h"
#include "data/enums/zone.h"
#include "data/yaml/merge.h"

#include <chrono>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fmt/format.h>
#include <optional>
#include <string>
#include <string_view>

namespace xi::data
{

inline auto zoneDataName(const xi::ZoneId zoneId, const std::string_view name) -> std::string
{
    return fmt::format("zones/{}/{}", EnumTraits<xi::ZoneId>::toName(zoneId), name);
}

inline auto zoneFilePath(const xi::ZoneId zoneId, const std::string_view name) -> std::string
{
    return fmt::format("data/{}.yaml", zoneDataName(zoneId, name));
}

// Per-zone data file. No core file means the zone declares none of this kind.
template <class Dataset>
auto loadZoneFile(const xi::ZoneId zoneId) -> std::optional<typename Dataset::Records>
{
    const auto path = zoneFilePath(zoneId, Dataset::kDataPath);
    if (!std::filesystem::exists(path))
    {
        return std::nullopt;
    }

    const auto modules = getDataModulePaths(zoneDataName(zoneId, Dataset::kDataPath), ".yaml");

    try
    {
        auto records = Dataset::decode(loadMergedYaml(path, modules));
        if constexpr (requires { Dataset::verifyZone(records, zoneId); })
        {
            Dataset::verifyZone(records, zoneId);
        }

        return records;
    }
    catch (const std::exception& error)
    {
        // Catch exceptions from workers, report the file and stop deliberately.
        ShowCriticalFmt("{} is not valid: {}", path, error.what());
        std::exit(-1);
    }
}

template <class Dataset>
auto loadDataset() -> typename Dataset::Records
{
    const auto dataPath = Dataset::kDataPath;
    const auto start    = std::chrono::steady_clock::now();
    const auto corePath = fmt::format("data/{}.yaml", dataPath);
    const auto modules  = getDataModulePaths(dataPath, ".yaml");

    try
    {
        auto       result       = Dataset::decode(loadMergedYaml(corePath, modules));
        const auto ms           = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
        const auto moduleSuffix = [&]() -> std::string
        {
            if (modules.empty())
            {
                return {};
            }

            return fmt::format("  +{} module{}", modules.size(), modules.size() == 1 ? "" : "s");
        }();
        ShowInfoFmt("[data] {} {} entries loaded in {}ms{}", dataPath, result.size(), ms, moduleSuffix);
        return result;
    }
    catch (const std::exception& error)
    {
        ShowErrorFmt("loadDataset({}) failed: {}", corePath, error.what());
        throw;
    }
}

} // namespace xi::data
