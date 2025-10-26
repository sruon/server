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

#include "action_result.h"

#include "enums/action/hit_distortion.h"
#include "enums/action/info.h"

ActionResult::ActionResult(const ResultSpec& spec)
: m_resolution(spec.resolution)
, m_kind(spec.kind)
, m_subKind(spec.subKind)
, m_info(spec.info)
, m_distortion(spec.distortion)
, m_knockback(spec.knockback)
, m_value(spec.value)
, m_message(spec.message)
, m_modifier(spec.modifier)
, m_proc(spec.proc)
, m_reaction(spec.reaction)
{
}

auto ActionResult::resolution() const -> ActionResolution
{
    return m_resolution;
}

auto ActionResult::kind() const -> uint8_t
{
    return m_kind;
}

auto ActionResult::subKind() const -> ActionSubKind
{
    return m_subKind;
}

auto ActionResult::info() const -> ActionResultInfo
{
    return m_info;
}

auto ActionResult::distortion() const -> HitDistortion
{
    return m_distortion;
}

auto ActionResult::knockback() const -> Knockback
{
    return m_knockback;
}

auto ActionResult::value() const -> int32_t
{
    return m_value;
}

auto ActionResult::message() const -> MSGBASIC_ID
{
    return m_message;
}

auto ActionResult::modifier() const -> ActionModifier
{
    return m_modifier;
}

auto ActionResult::hasProc() const -> bool
{
    return m_proc.has_value();
}

auto ActionResult::proc() const -> const ProcSpec&
{
    return m_proc.value();
}

auto ActionResult::hasReaction() const -> bool
{
    return m_reaction.has_value();
}

auto ActionResult::reaction() const -> const ReactSpec&
{
    return m_reaction.value();
}

auto ActionResult::setResolution(const ActionResolution res) -> ActionResult&
{
    m_resolution = res;

    return *this;
}

auto ActionResult::setValue(const int32_t val) -> ActionResult&
{
    m_value = val;

    return *this;
}

auto ActionResult::setMessage(const MSGBASIC_ID msg) -> ActionResult&
{
    m_message = msg;

    return *this;
}

auto ActionResult::setModifier(const ActionModifier mod) -> ActionResult&
{
    m_modifier = mod;

    return *this;
}

auto ActionResult::setSubKind(const ActionSubKind sk) -> ActionResult&
{
    m_subKind = sk;

    return *this;
}

auto ActionResult::setKind(const uint8_t k) -> ActionResult&
{
    m_kind = k;

    return *this;
}

auto ActionResult::setInfo(const ActionResultInfo info) -> ActionResult&
{
    m_info = info;

    return *this;
}

auto ActionResult::setDistortion(const HitDistortion dist) -> ActionResult&
{
    m_distortion = dist;

    return *this;
}

auto ActionResult::setKnockback(const Knockback kb) -> ActionResult&
{
    m_knockback = kb;

    return *this;
}

auto ActionResult::asCritical(const int32_t damage, const MSGBASIC_ID message) -> void
{
    m_message    = message;
    m_info       = ActionInfo::CriticalHit;
    m_distortion = HitDistortion::Heavy;
    m_value      = damage;
}

auto ActionResult::asAbsorb(const int32_t damage) -> void
{
    m_value = std::abs(damage);
}

auto ActionResult::asShadowAbsorb() -> void
{
    m_resolution = ActionResolution::Miss;
    m_message    = MSGBASIC_SHADOW_ABSORB;
    m_value      = 1;
}

auto ActionResult::addProc(const ProcSpec& proc) -> void
{
    m_proc = proc;
}

auto ActionResult::addReaction(const ReactSpec& react) -> void
{
    m_reaction = react;
}
