/*
===========================================================================

  Copyright (c) 2025 LandSandBoat Dev Teams

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

#include "in_memory_sink.h"
#include "common/logging.h"
#include "common/settings.h"

#include <spdlog/async.h>

// Replace all loggers sinks with the in-memory sink
void InMemorySink::capture(const std::shared_ptr<InMemorySink>& sink)
{
    const auto loggerNames = std::vector<std::string>{
        "critical", "error", "lua", "warn", "info", "debug", "trace"
    };

    // spdlog blows up if we don't!
    spdlog::shutdown();

    // Re-register all loggers as SYNCHRONOUS loggers with only our in-memory sink
    // This ensures all logs are immediately written to the sink
    for (const auto& name : loggerNames)
    {
        const auto logger = std::make_shared<spdlog::logger>(name, sink);
        logger->set_level(spdlog::level::trace);
        spdlog::register_logger(logger);
    }

    // The loggers were just dropped and re-created, so the lock-free loggerFor cache now holds
    // dangling pointers. Re-resolve it before anything logs again.
    logging::RefreshLoggerCache();

    logging::SetPattern(settings::get<std::string>("test.PATTERN"));
}
