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
#include "action_target.h"
#include "entities/baseentity.h"
#include "enums/action/category.h"
#include "enums/four_cc.h"
#include <variant>

class CSpell;
enum class SpellID : uint16;
class ActionTarget;
class CWeaponSkill;
class CAbility;
class CMobSkill;
class CPetSkill;
class CItemUsable;
enum class WeaknessType;

using ActionArgument = std::variant<uint32_t, FourCC, SpellID>;

struct ActionSpec
{
    ActionCategory category;
    ActionArgument argument{};
    uint32_t       info{ 0 };
};

class Action
{
public:
    explicit Action(CBaseEntity* PActor, const ActionSpec& spec);

    // Accessors
    auto actorId() const -> uint32_t;
    auto category() const -> ActionCategory;
    auto argument() const -> ActionArgument;
    auto info() const -> uint32_t;
    auto recast() const -> std::chrono::seconds;
    auto targets() const -> const std::vector<ActionTarget>&;

    // Modifiers
    auto setInfo(uint32_t info) -> Action&;
    auto setRecast(std::chrono::seconds recast) -> Action&;
    auto setRecast(uint32_t recast) -> Action&;
    auto setRecast(timer::duration recast) -> Action&;
    auto setCategory(ActionCategory category) -> Action&;
    auto setArgument(ActionArgument arg) -> Action&;

    // Add target without results (for chaining with addResult)
    auto addTarget(uint32_t targetId) -> ActionTarget&;
    auto addTarget(const CBaseEntity* PTarget) -> ActionTarget&;

    // Single result per target
    auto addTarget(uint32_t targetId, const ResultSpec& result) -> ActionTarget&;
    auto addTarget(const CBaseEntity* PTarget, const ResultSpec& result) -> ActionTarget&;

    // Get target reference for advanced manipulation
    auto target(uint32_t targetId) -> ActionTarget&;
    auto target(const CBaseEntity* PEntity) -> ActionTarget&;

    // Direct target access
    auto target(size_t index) -> ActionTarget&;
    auto target(size_t index) const -> const ActionTarget&;

    // Convenience: access first target
    auto target() -> ActionTarget&;
    auto target() const -> const ActionTarget&;

    auto targetCount() const -> size_t;

    template <typename Func>
    void ForAllTargets(Func&& func)
    {
        std::ranges::for_each(targets(), std::forward<Func>(func));
    }

    template <typename Func>
    void ForAllResults(Func&& func)
    {
        for (auto& target : targets())
        {
            std::ranges::for_each(target.results(), std::forward<Func>(func));
        }
    }

private:
    CBaseEntity*                            m_actor;
    ActionCategory                          m_category;
    std::variant<uint32_t, FourCC, SpellID> m_argument;
    uint32_t                                m_info;
    std::vector<ActionTarget>               m_targets;

    auto findTarget(uint32_t targetId) -> ActionTarget*;
};

namespace Actions
{
    // cmd_no = 1
    auto BasicAttack(CBaseEntity* PActor) -> Action;

    // cmd_no = 2
    auto RangedFinish(CBaseEntity* PActor) -> Action;

    // cmd_no = 3
    auto SkillFinish(CBaseEntity* PActor, const CWeaponSkill* PSkill) -> Action;

    // cmd_no = 4
    auto MagicFinish(CBaseEntity* PActor, CSpell* PSpell) -> Action;

    // cmd_no = 5
    auto ItemFinish(CBaseEntity* PActor, const CItemUsable* PItem) -> Action;

    // cmd_no = 6
    auto AbilityFinish(CBaseEntity* PActor, const CAbility* PAbility) -> Action;

    // cmd_no = 7
    auto SkillStart(CBaseEntity* PActor, const CAbility* PAbility, const CBaseEntity* PTarget) -> Action;
    auto SkillStart(CBaseEntity* PActor, const CWeaponSkill* PSkill, const CBaseEntity* PTarget) -> Action;
    auto SkillInterrupt(CBaseEntity* PActor) -> Action;
    auto MobSkillStart(CBaseEntity* PActor, const CMobSkill* PSkill, const CBaseEntity* PTarget, ActionCategory category, MSGBASIC_ID message) -> Action;

    // cmd_no = 8
    auto MagicStart(CBaseEntity* PActor, CSpell* PSpell, const CBaseEntity* PTarget) -> Action;
    auto MagicInterrupt(CBaseEntity* PActor, const CSpell* PSpell) -> Action;

    // cmd_no = 9
    auto ItemStart(CBaseEntity* PActor, const CItemUsable* PItem, const CBaseEntity* PTarget, MSGBASIC_ID message = MSGBASIC_ITEM_USE) -> Action;
    auto ItemInterrupt(CBaseEntity* PActor) -> Action;

    // cmd_no = 10
    auto AbilityInterrupt(CBaseEntity* PActor) -> Action;

    // cmd_no = 11
    auto MobSkillFinish(CBaseEntity* PActor, const CMobSkill* PSkill) -> Action;

    // cmd_no = 12
    auto RangedStart(CBaseEntity* PActor, const CBaseEntity* PTarget) -> Action;
    auto RangedInterrupt(CBaseEntity* PActor, const CBaseEntity* PTarget) -> Action;

    // cmd_no = 13
    // ???

    // cmd_no = 14
    // TODO: Dancer

    // cmd_no = 15
    // TODO: RUN
    auto PetSkillFinish(CBaseEntity* PActor, const CPetSkill* PSkill) -> Action;
    auto PetSkillStart(CBaseEntity* PActor, const CPetSkill* PSkill, const CBaseEntity* PTarget) -> Action;

    auto WeaknessTrigger(CBaseEntity* PActor, WeaknessType weaknessType) -> Action;
} // namespace Actions
