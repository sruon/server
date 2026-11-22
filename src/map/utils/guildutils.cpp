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

#include "guildutils.h"

#include "common/logging.h"
#include "common/vana_time.h"

#include <vector>

#include "charutils.h"
#include "data/datasets/guilds/dataset.h"
#include "dataset_loader.h"
#include "guild.h"
#include "item_container.h"
#include "itemutils.h"
#include "serverutils.h"

namespace
{

std::vector<CGuild*> g_PGuildList;

} // namespace

namespace guildutils
{

void Initialize()
{
    const auto guilds = xi::data::loadDataset<xi::data::datasets::guilds::Dataset>();
    g_PGuildList.reserve(guilds.size());

    for (const auto& guild : guilds)
    {
        g_PGuildList.emplace_back(new CGuild(guild));
    }

    UpdateGuildPointsPattern();
}

void Cleanup()
{
    // Delete pointers and cleanup vectors manually
    for (auto guild : g_PGuildList)
    {
        destroy(guild);
    }

    g_PGuildList.clear();
}

void UpdateGuildPointsPattern()
{
    // TODO: This function can be faulty when dealing with multiple processes. Needs to be synchronized properly across servers.
    const auto jstDayOfYear = earth_time::jst::get_yearday();
    const bool doUpdate     = serverutils::GetServerVar("[GUILD]pattern_update") != jstDayOfYear;

    uint8 pattern = xirand::GetRandomNumber(8);
    if (doUpdate)
    {
        // write the new pattern and update time to try to prevent other servers from updating the pattern
        serverutils::SetServerVar("[GUILD]pattern_update", jstDayOfYear);
        serverutils::SetServerVar("[GUILD]pattern", pattern);
    }
    else
    {
        // load the pattern in case it was set by another server (and this server did not set it)
        pattern = serverutils::GetServerVar("[GUILD]pattern");
    }

    for (const auto PGuild : g_PGuildList)
    {
        PGuild->updateGuildPointsPattern(pattern);
    }

    ShowDebug("Guild point pattern update has finished. New pattern: %d", pattern);
}

auto GetGuild(const uint8 guildId) -> CGuild*
{
    if (guildId < g_PGuildList.size())
    {
        return g_PGuildList.at(guildId);
    }

    ShowDebug("Guild with id <%u> is not found on server", guildId);
    return nullptr;
}

} // namespace guildutils
