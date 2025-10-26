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

#include "action_target.h"

#include "ability.h"

ActionTarget::ActionTarget(const uint32_t targetId)
: m_targetId(targetId)
{
}

auto ActionTarget::targetId() const -> uint32_t
{
    return m_targetId;
}

auto ActionTarget::setTargetId(const uint32_t targetId) -> void
{
    m_targetId = targetId;
}

auto ActionTarget::results() const -> const std::vector<ActionResult>&
{
    return m_results;
}

auto ActionTarget::addResult(const ResultSpec& spec) -> ActionResult&
{
    auto& result = m_results.emplace_back(spec);

    // Negative number means the target is absorbing, swap the message to its Absorb equivalent.
    if (spec.value < 0)
    {
        result.setMessage(ability::GetAbsorbMessage(spec.message));
        result.setValue(-spec.value);
    }

    return result;
}

auto ActionTarget::result(const size_t index) -> ActionResult&
{
    return m_results.at(index);
}

auto ActionTarget::result(const size_t index) const -> const ActionResult&
{
    return m_results.at(index);
}

auto ActionTarget::result() -> ActionResult&
{
    return m_results.at(0);
}

auto ActionTarget::result() const -> const ActionResult&
{
    return m_results.at(0);
}

auto ActionTarget::resultCount() const -> size_t
{
    return m_results.size();
}
