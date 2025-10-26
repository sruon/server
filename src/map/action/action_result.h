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

#include "action_proc.h"
#include "action_react.h"
#include "common/cbasetypes.h"
#include "enums/action/modifier.h"
#include "enums/action/resolution.h"
#include "enums/msg_basic.h"

#include <variant>

enum class Knockback : uint8_t;
enum class HitDistortion : uint8_t;
enum class RuneElement : uint8_t;
enum class ActionInfo : uint8_t;
enum class AttackAnimation;
enum class ActionAnimation : uint16_t;
using ActionSubKind    = std::variant<ActionAnimation, AttackAnimation>;
using ActionResultInfo = std::variant<ActionInfo, RuneElement>;

struct ResultSpec
{
    // Resolution (hit, miss, evade, etc.)
    ActionResolution resolution{ ActionResolution::Hit };

    // Kind and subkind (for animation/effect type)
    uint8_t       kind{ 0 };
    ActionSubKind subKind{};

    // Info flags
    ActionResultInfo info{};

    // Distortion and knockback
    HitDistortion distortion{};
    Knockback     knockback{};

    // Damage/healing value
    int32_t value{ 0 };

    // Message to display
    MSGBASIC_ID message{ MSGBASIC_NONE };

    // Modifier bits
    ActionModifier modifier{ ActionModifier::None };

    // Optional proc and reaction
    std::optional<ProcSpec>  proc{};
    std::optional<ReactSpec> reaction{};
};

class ActionResult
{
public:
    explicit ActionResult(const ResultSpec& spec);

    // Accessors
    auto resolution() const -> ActionResolution;
    auto kind() const -> uint8_t;
    auto subKind() const -> std::variant<ActionAnimation, AttackAnimation>;
    auto info() const -> ActionResultInfo;
    auto distortion() const -> HitDistortion;
    auto knockback() const -> Knockback;
    auto value() const -> int32_t;
    auto message() const -> MSGBASIC_ID;
    auto modifier() const -> ActionModifier;

    auto hasProc() const -> bool;
    auto proc() const -> const ProcSpec&;

    auto hasReaction() const -> bool;
    auto reaction() const -> const ReactSpec&;

    // Modifiers
    auto setResolution(ActionResolution res) -> ActionResult&;
    auto setValue(int32_t val) -> ActionResult&;
    auto setMessage(MSGBASIC_ID msg) -> ActionResult&;
    auto setModifier(ActionModifier mod) -> ActionResult&;
    auto setSubKind(ActionSubKind sk) -> ActionResult&;
    auto setKind(uint8_t k) -> ActionResult&;
    auto setInfo(ActionResultInfo info) -> ActionResult&;
    auto setDistortion(HitDistortion dist) -> ActionResult&;
    auto setKnockback(Knockback kb) -> ActionResult&;

    // Common pattern helpers
    auto asCritical(int32_t damage, MSGBASIC_ID message = MSGBASIC_ATTACK_CRIT) -> void;
    auto asAbsorb(int32_t damage) -> void;
    auto asShadowAbsorb() -> void;

    auto addProc(const ProcSpec& proc) -> void;
    auto addReaction(const ReactSpec& react) -> void;

private:
    ActionResolution         m_resolution;
    uint8_t                  m_kind;
    ActionSubKind            m_subKind;
    ActionResultInfo         m_info;
    HitDistortion            m_distortion;
    Knockback                m_knockback;
    int32_t                  m_value;
    MSGBASIC_ID              m_message;
    ActionModifier           m_modifier;
    std::optional<ProcSpec>  m_proc;
    std::optional<ReactSpec> m_reaction;
};

namespace Results
{
    inline auto Hit(const int32_t damage, const MSGBASIC_ID msg = MSGBASIC_ATTACK_HITS) -> ResultSpec
    {
        return {
            .resolution = ActionResolution::Hit,
            .value      = damage,
            .message    = msg,
        };
    }

    inline auto Critical(const int32_t damage, const MSGBASIC_ID msg = MSGBASIC_ATTACK_CRIT) -> ResultSpec
    {
        return {
            .resolution = ActionResolution::Hit,
            .value      = damage,
            .message    = msg,
        };
    }

    inline auto Heal(const int32_t amount, const MSGBASIC_ID msg = MSGBASIC_MAGIC_RECOVERS_HP) -> ResultSpec
    {
        return {
            .resolution = ActionResolution::Hit,
            .value      = amount,
            .message    = msg,
        };
    }

    inline auto Miss(const MSGBASIC_ID msg = MSGBASIC_ATTACK_MISSES) -> ResultSpec
    {
        return {
            .resolution = ActionResolution::Miss,
            .message    = msg,
        };
    }

    inline auto Evade(const MSGBASIC_ID msg = MSGBASIC_TARGET_EVADES) -> ResultSpec
    {
        return {
            .resolution = ActionResolution::Miss,
            .message    = msg,
        };
    }

    inline auto Parry(const MSGBASIC_ID msg = MSGBASIC_TARGET_PARRIES) -> ResultSpec
    {
        return {
            .resolution = ActionResolution::Parry,
            .message    = msg,
        };
    }

    inline auto Anticipate(const MSGBASIC_ID msg = MSGBASIC_TARGET_ANTICIPATES) -> ResultSpec
    {
        return {
            .resolution = ActionResolution::Miss,
            .message    = msg,
        };
    }

    inline auto Resist(const MSGBASIC_ID msg = MSGBASIC_MAGIC_RESISTED) -> ResultSpec
    {
        return {
            .resolution = ActionResolution::Hit,
            .message    = msg,
        };
    }

    inline auto Status(const MSGBASIC_ID msg, const ActionResolution res = ActionResolution::Hit) -> ResultSpec
    {
        return {
            .resolution = res,
            .message    = msg,
        };
    }

} // namespace Results
