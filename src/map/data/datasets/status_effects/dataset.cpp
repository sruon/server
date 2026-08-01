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

#include "data/datasets/status_effects/dataset.h"

#include "data/datasets/status_effects/yaml.h"
#include "data/yaml/read.h"

#include <fmt/format.h>

#include <stdexcept>
#include <utility>

namespace xi::data::datasets::status_effects
{

static_assert(glz::reflect<wire::StatusEffect>::size == 12,
              "update the status-effect resolver when the wire shape changes");
static_assert(glz::reflect<StatusEffectData>::size == 12,
              "update the status-effect resolver when the domain shape changes");

// Decode and index status effects by numeric ID.
auto Dataset::decode(const std::string_view text) -> Records
{
    const auto file = yaml::read<YamlFile>(text);
    Records    result;

    for (const auto& [key, input] : file.status_effects)
    {
        yaml::verifyNamedMapEntry<xi::StatusEffect>(key, input.id);
        StatusEffectData value{
            .Id               = input.id,
            .Name             = input.name.value_or(key),
            .Flags            = yaml::resolveFlags(input.flags),
            .ExclusionGroup   = yaml::resolveEnum(input.exclusion_group),
            .Negative         = yaml::resolveEnum(input.negative),
            .Overwrite        = yaml::resolveEnum(input.overwrite),
            .Block            = yaml::resolveEnum(input.block),
            .Remove           = yaml::resolveEnum(input.remove),
            .Element          = yaml::resolveEnum(input.element),
            .MinDuration      = input.min_duration.value_or(0),
            .SortKey          = input.sort_key.value_or(0),
            .WearOffMessageId = input.wear_off_message_id.value_or(0),
        };

        if (!result.try_emplace(value.Id, std::move(value)).second)
        {
            throw std::runtime_error(fmt::format("duplicate status effect id {}", input.id));
        }
    }
    return result;
}

} // namespace xi::data::datasets::status_effects
