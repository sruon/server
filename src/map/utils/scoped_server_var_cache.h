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

#include <unordered_set>

namespace serverutils
{

class ScopedServerVarCache
{
public:
    ScopedServerVarCache();
    ~ScopedServerVarCache();

    DISALLOW_COPY_AND_MOVE(ScopedServerVarCache);

private:
    HashMap<std::string, std::pair<int32, uint32>> cache_;
    std::unordered_set<std::string>                changes_;
};

} // namespace serverutils
