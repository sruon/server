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

#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace xi::data::shared
{

using Names = std::variant<std::string, std::vector<std::string>>;

inline auto toNames(const std::optional<Names>& source) -> std::vector<std::string>
{
    if (!source)
    {
        return {};
    }

    if (const auto* one = std::get_if<std::string>(&*source))
    {
        return { *one };
    }

    return std::get<std::vector<std::string>>(*source);
}

} // namespace xi::data::shared
