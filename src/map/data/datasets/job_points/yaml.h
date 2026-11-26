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

#include "common/cbasetypes.h"
#include "data/enums/job.h"
#include "data/enums/mod.h"
#include "data/yaml/enum_keyed_map.h"
#include "data/yaml/schema_annotations.h"

#include <glaze/glaze.hpp>

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace xi::data::datasets::job_points::wire
{

struct Category
{
    uint16 id{};
};

inline constexpr std::string_view kSpellsKey{ "spells" };

// mods by name (`def: 10`) and the spells learned (`spells: [death]`)
struct GiftTier
{
    std::map<std::string, std::variant<int16, std::vector<std::string>>> Entries;
};

using Gifts = std::map<uint16, GiftTier>;

struct Job
{
    std::map<std::string, Category> categories;
    std::optional<Gifts>            gifts;
};

struct Document
{
    yaml::EnumKeyedMap<xi::Job, Job> job_points;

    using YamlRoot = yaml::DatasetRoot<&Document::job_points>;
};

} // namespace xi::data::datasets::job_points::wire

template <>
struct glz::json_schema<xi::data::datasets::job_points::wire::Category>
{
    glz::schema id{ .description = "Client-facing job point ID: job ID * 32 + slot 0-9.", .minimum = 32L, .maximum = 713L };
};

template <>
struct glz::json_schema<xi::data::datasets::job_points::wire::Job>
{
    glz::schema categories{ .description = "Job point categories keyed by canonical name, in ID order." };
    glz::schema gifts{ .description = "Mods granted at level 99 and spells learned, keyed by job points spent. Spells are spell_list names; missing ones are skipped." };
};

template <>
struct glz::json_schema<xi::data::datasets::job_points::wire::Document>
{
    glz::schema job_points{ .description = "Job point categories and gifts by job." };
};

template <>
struct glz::meta<xi::data::datasets::job_points::wire::GiftTier>
{
    using T                     = xi::data::datasets::job_points::wire::GiftTier;
    static constexpr auto name  = "JobPointGiftTier";
    static constexpr auto value = &T::Entries;
};

namespace glz::detail
{

template <>
struct to_json_schema<xi::data::datasets::job_points::wire::GiftTier>
{
    template <auto Opts>
    static void op(auto& s, auto& defs)
    {
        schema amount{};
        to_json_schema<int16>::template op<Opts>(amount, defs);

        schema spells{};
        to_json_schema<std::vector<std::string>>::template op<Opts>(spells, defs);

        std::map<std::string_view, schema, std::less<>> properties;
        for (const auto& [name, _] : xi::data::EnumTraits<xi::Mod>::kEntries)
        {
            properties.emplace(name, amount);
        }

        properties.emplace(xi::data::datasets::job_points::wire::kSpellsKey, spells);

        s.type                 = std::string_view{ "object" };
        s.properties           = std::move(properties);
        s.additionalProperties = false;
    }
};

} // namespace glz::detail
