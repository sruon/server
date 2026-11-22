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
#include "common/types/hash_map.h"
#include "data/enums/mod.h"
#include "data/enums/pet_mod_type.h"

#include <optional>
#include <string_view>

namespace xi::data
{

// Each point of augment value moves every mod one further from zero, then the total is scaled by Multiplier.
struct AugmentData
{
    HashMap<xi::Mod, int16>       Mods{};
    std::optional<xi::PetModType> Pet{};
    uint16                        Multiplier{ 1 };
};

using Augments = HashMap<uint16, AugmentData>;

} // namespace xi::data

namespace xi::data::datasets::augments::wire
{

struct Document;

}

namespace xi::data::datasets::augments
{

// Augment IDs are 11 bits in item exdata.
constexpr uint16 kMaxAugmentId = 2047;

struct Dataset
{
    using Records      = Augments;
    using YamlDocument = wire::Document;

    static constexpr std::string_view kDataPath{ "augments" };
    static constexpr std::string_view kTitle{ "Augments" };
    static constexpr std::string_view kDescription{ "Modifiers granted by standard augment IDs." };

    static auto decode(std::string_view text) -> Records;
};

} // namespace xi::data::datasets::augments
