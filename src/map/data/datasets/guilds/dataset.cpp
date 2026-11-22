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

#include "data/datasets/guilds/dataset.h"

#include "common/enum_traits.h"
#include "data/datasets/guilds/yaml.h"
#include "data/yaml/read.h"

#include <fmt/format.h>

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace xi::data::datasets::guilds
{

namespace
{

auto toPattern(const wire::Pattern& source, const std::string_view where) -> GuildPointPattern
{
    if (source.items.empty())
    {
        throw std::runtime_error(fmt::format("{} requests no items", where));
    }

    GuildPointPattern pattern{ .MaxPoints = source.max_points };
    for (const auto& [item, points] : source.items)
    {
        if (points == 0)
        {
            throw std::runtime_error(fmt::format("{} item '{}' is worth no points", where, item));
        }

        pattern.Items.emplace_back(GuildPointItem{ .Item = item, .Points = points });
    }

    return pattern;
}

} // namespace

auto Dataset::decode(const std::string_view text) -> Records
{
    const auto document = yaml::read<YamlDocument>(text);

    Records records;
    for (const auto& [guildName, source] : document.guilds.Values)
    {
        if (source.currency.empty())
        {
            throw std::runtime_error(fmt::format("guild {} has no currency", guildName));
        }

        GuildData record{
            .Id       = EnumTraits<xi::Guild>::fromName(guildName),
            .Currency = source.currency,
        };

        for (const auto& [rankName, patterns] : source.ranks.Values)
        {
            const auto where = fmt::format("guild {} rank {}", guildName, rankName);
            record.Ranks.emplace(EnumTraits<xi::CraftRank>::fromName(rankName),
                                 GuildPointPatterns{
                                     toPattern(patterns.a, fmt::format("{} pattern a", where)),
                                     toPattern(patterns.b, fmt::format("{} pattern b", where)),
                                     toPattern(patterns.c, fmt::format("{} pattern c", where)),
                                     toPattern(patterns.d, fmt::format("{} pattern d", where)),
                                     toPattern(patterns.e, fmt::format("{} pattern e", where)),
                                     toPattern(patterns.f, fmt::format("{} pattern f", where)),
                                     toPattern(patterns.g, fmt::format("{} pattern g", where)),
                                     toPattern(patterns.h, fmt::format("{} pattern h", where)),
                                 });
        }

        records.emplace_back(std::move(record));
    }

    std::ranges::sort(records, {}, &GuildData::Id);
    for (const auto& [guild, id] : EnumTraits<xi::Guild>::kEntries)
    {
        if (!std::ranges::contains(records, id, &GuildData::Id))
        {
            throw std::runtime_error(fmt::format("guild {} is missing", guild));
        }
    }

    return records;
}

} // namespace xi::data::datasets::guilds
