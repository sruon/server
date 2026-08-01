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

#include <glaze/glaze.hpp>

#include <string_view>

namespace xi::data::yaml
{

inline constexpr std::string_view kNamedMapSchemaMarker{ "lsb:named-map" };
inline constexpr std::string_view kNamedMapKeyPattern{ "^[a-z][a-z0-9_]*$" };

// Keep generator metadata visible to Glaze without making it part of the wire type.
struct DatasetRootSchema
{
    // Require the dataset payload while allowing metadata to stay optional.
    static constexpr bool requires_key(const std::string_view key, const bool isNullable)
    {
        return key != "meta" && !isNullable;
    }
};

// Mark a named map so the schema pass can constrain its keys.
inline auto namedMapDoc(const std::string_view description) -> glz::schema
{
    return { .title = kNamedMapSchemaMarker, .description = description };
}

} // namespace xi::data::yaml
