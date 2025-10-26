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

#pragma once

#include "action_result.h"
#include "common/cbasetypes.h"

struct ResultSpec;
class ActionTarget
{
public:
    explicit ActionTarget(uint32_t targetId);

    auto targetId() const -> uint32_t;
    auto setTargetId(uint32_t targetId) -> void;
    auto results() const -> const std::vector<ActionResult>&;

    auto addResult(const ResultSpec& spec) -> ActionResult&;

    auto result(size_t index) -> ActionResult&;
    auto result(size_t index) const -> const ActionResult&;
    auto result() -> ActionResult&;
    auto result() const -> const ActionResult&;

    auto resultCount() const -> size_t;

private:
    uint32_t                  m_targetId;
    std::vector<ActionResult> m_results;
};
