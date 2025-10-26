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

#include "action.h"

#include "ability.h"
#include "enums/action/animation.h"
#include "items/item_usable.h"
#include "mobskill.h"
#include "petskill.h"
#include "spell.h"
#include "utils/mobutils.h"
#include "weapon_skill.h"

Action::Action(CBaseEntity* PActor, const ActionSpec& spec)
: m_actor(PActor)
, m_category(spec.category)
, m_argument(spec.argument)
, m_info(spec.info)
{
}

auto Action::actorId() const -> uint32_t
{
    return m_actor->id;
}

auto Action::category() const -> ActionCategory
{
    return m_category;
}

auto Action::argument() const -> ActionArgument
{
    return m_argument;
}

auto Action::info() const -> uint32_t
{
    return m_info;
}

auto Action::recast() const -> std::chrono::seconds
{
    return std::chrono::seconds(m_info);
}

auto Action::targets() const -> const std::vector<ActionTarget>&
{
    return m_targets;
}

auto Action::setInfo(const uint32_t info) -> Action&
{
    m_info = info;

    return *this;
}

auto Action::setRecast(const std::chrono::seconds recast) -> Action&
{
    m_info = timer::count_seconds(recast);

    return *this;
}

auto Action::setRecast(const timer::duration recast) -> Action&
{
    m_info = timer::count_seconds(recast);

    return *this;
}

auto Action::setRecast(const uint32_t recast) -> Action&
{
    m_info = recast;

    return *this;
}

auto Action::setCategory(const ActionCategory category) -> Action&
{
    m_category = category;

    return *this;
}

auto Action::setArgument(const ActionArgument arg) -> Action&
{
    m_argument = arg;
    return *this;
}

auto Action::target(const size_t index) -> ActionTarget&
{
    return m_targets.at(index);
}

auto Action::target(const size_t index) const -> const ActionTarget&
{
    return m_targets.at(index);
}

auto Action::target() -> ActionTarget&
{
    return m_targets.at(0);
}

auto Action::target() const -> const ActionTarget&
{
    return m_targets.at(0);
}

auto Action::targetCount() const -> size_t
{
    return m_targets.size();
}

auto Action::addTarget(uint32_t targetId) -> ActionTarget&
{
    auto* target = findTarget(targetId);
    if (!target)
    {
        target = &m_targets.emplace_back(targetId);
    }

    return *target;
}

auto Action::addTarget(const CBaseEntity* PTarget) -> ActionTarget&
{
    return addTarget(PTarget->id);
}

auto Action::addTarget(uint32_t targetId, const ResultSpec& result) -> ActionTarget&
{
    auto* target = findTarget(targetId);
    if (!target)
    {
        target = &m_targets.emplace_back(targetId);
    }
    target->addResult(result);

    return *target;
}

auto Action::addTarget(const CBaseEntity* PTarget, const ResultSpec& result) -> ActionTarget&
{
    return addTarget(PTarget->id, result);
}

auto Action::target(uint32_t targetId) -> ActionTarget&
{
    if (auto* existingTarget = findTarget(targetId))
    {
        return *existingTarget;
    }

    return m_targets.emplace_back(targetId);
}

auto Action::target(const CBaseEntity* PEntity) -> ActionTarget&
{
    return target(PEntity->id);
}

auto Action::findTarget(const uint32_t targetId) -> ActionTarget*
{
    for (auto& target : m_targets)
    {
        if (target.targetId() == targetId)
        {
            return &target;
        }
    }

    return nullptr;
}

namespace Actions
{
    auto MagicStart(CBaseEntity* PActor, CSpell* PSpell, const CBaseEntity* PTarget) -> Action
    {
        MSGBASIC_ID messageId = PActor->objtype != TYPE_PC ? MSGBASIC_STARTS_CASTING_SELF : MSGBASIC_STARTS_CASTING_TARGET;
        if (PSpell->getFlag() & SPELLFLAG_NO_START_MSG)
        {
            messageId = MSGBASIC_NONE;
        }

        Action action(PActor,
                      {
                          .category = ActionCategory::MagicStart,
                          .argument = PSpell->getFourCC(),
                      });

        action.addTarget(PTarget,
                         {
                             .value   = static_cast<int32_t>(PSpell->getID()),
                             .message = messageId,
                         });

        return action;
    }

    auto SkillStart(CBaseEntity* PActor, const CAbility* PAbility, const CBaseEntity* PTarget) -> Action
    {
        auto action = Action(PActor,
                             {
                                 .category = ActionCategory::SkillStart,
                                 .argument = PAbility->getID(),
                             });

        action.addTarget(PTarget,
                         {
                             .subKind = ActionAnimation::SkillStart,
                             .value   = static_cast<int32_t>(PAbility->getID()),
                             .message = MSGBASIC_READIES_SKILL,
                         });

        return action;
    }

    auto SkillStart(CBaseEntity* PActor, const CWeaponSkill* PSkill, const CBaseEntity* PTarget) -> Action
    {
        Action action(PActor,
                      {
                          .category = ActionCategory::SkillStart,
                          .argument = PSkill->getID(),
                      });

        action.addTarget(PTarget,
                         {
                             .subKind = ActionAnimation::SkillStart,
                             .value   = static_cast<int32_t>(PSkill->getID()),
                             .message = MSGBASIC_READIES_WS,
                         });

        return action;
    }

    auto SkillFinish(CBaseEntity* PActor, const CWeaponSkill* PSkill) -> Action
    {
        Action action(PActor,
                      {
                          .category = ActionCategory::SkillFinish,
                          .argument = PSkill->getID(),
                      });
        return action;
    }

    auto SkillInterrupt(CBaseEntity* PActor) -> Action
    {
        Action action(PActor, {
                                  .category = ActionCategory::SkillStart,
                                  .argument = FourCC::SkillInterrupt,
                              });

        action.addTarget(PActor,
                         {
                             .resolution = ActionResolution::Miss,
                             .subKind    = ActionAnimation::SkillInterrupt,
                         });

        return action;
    }

    auto ItemInterrupt(CBaseEntity* PActor) -> Action
    {
        Action action(PActor,
                      {
                          .category = ActionCategory::ItemStart,
                          .argument = FourCC::ItemInterrupt,
                      });

        action.addTarget(PActor,
                         {
                             .subKind = ActionAnimation::ItemInterrupt,
                         });

        return action;
    }

    auto AbilityInterrupt(CBaseEntity* PActor) -> Action
    {
        Action action(PActor,
                      {
                          .category = ActionCategory::MagicFinish,
                          .argument = FourCC::SkillInterrupt,
                      });

        action.addTarget(PActor,
                         {
                             .subKind = ActionAnimation::SkillInterrupt,
                         });

        return action;
    }

    auto MobSkillStart(CBaseEntity* PActor, const CMobSkill* PSkill, const CBaseEntity* PTarget, const ActionCategory category, const MSGBASIC_ID message) -> Action
    {
        Action action(PActor,
                      {
                          .category = category,
                          .argument = PSkill->getID(),
                      });

        action.addTarget(PTarget,
                         {
                             .value   = static_cast<int32_t>(PSkill->getID()),
                             .message = message,
                         });

        return action;
    }

    auto PetSkillStart(CBaseEntity* PActor, const CPetSkill* PSkill, const CBaseEntity* PTarget) -> Action
    {
        Action action(PActor,
                      {
                          .category = ActionCategory::SkillStart,
                      });

        if (PSkill->getMobSkillID() > 0)
        {
            action.addTarget(PTarget,
                             {
                                 .resolution = ActionResolution::Hit,
                                 .subKind    = ActionAnimation::PetSkillStart,
                                 .value      = static_cast<int32_t>(PSkill->getMobSkillID()),
                                 .message    = MSGBASIC_READIES_WS,
                             });
        }
        else
        {
            action.addTarget(PTarget,
                             {
                                 .resolution = ActionResolution::Hit,
                                 .value      = static_cast<int32_t>(PSkill->getID()),
                                 .message    = MSGBASIC_READIES_SKILL,
                             });
        }

        return action;
    }

    auto ItemStart(CBaseEntity* PActor, const CItemUsable* PItem, const CBaseEntity* PTarget, const MSGBASIC_ID message) -> Action
    {
        Action action(PActor,
                      {
                          .category = ActionCategory::ItemStart,
                          .argument = PItem->getID(),
                      });

        action.addTarget(PTarget,
                         {
                             .value   = static_cast<int32_t>(PItem->getID()),
                             .message = message,
                         });

        return action;
    }

    auto RangedStart(CBaseEntity* PActor, const CBaseEntity* PTarget) -> Action
    {
        // Determine animation from weapon
        // auto        animation = ANIMATION_RANGED_MARKSMANSHIP;
        // const auto* weapon    = dynamic_cast<CItemWeapon*>(PActor->m_Weapons[SLOT_RANGED]);
        // if (weapon && weapon->getSubSkillType() == SUBSKILL_LONGBOW)
        // {
        //     animation = ANIMATION_RANGED_ARCHERY;
        // }

        Action action(PActor,
                      {
                          .category = ActionCategory::RangedStart,
                          .argument = FourCC::RangeStart,
                      });

        // Add target with animation
        action.addTarget(PTarget,
                         {
                             .subKind = static_cast<ActionAnimation>(ANIMATION_RANGED_MARKSMANSHIP),
                         });

        return action;
    }

    auto RangedInterrupt(CBaseEntity* PActor, const CBaseEntity* PTarget) -> Action
    {
        // Determine animation from weapon
        // auto        animation = ANIMATION_RANGED_MARKSMANSHIP;
        // const auto* weapon    = dynamic_cast<CItemWeapon*>(PActor->m_Weapons[SLOT_RANGED]);
        // if (weapon && weapon->getSubSkillType() == SUBSKILL_LONGBOW)
        // {
        //     animation = ANIMATION_RANGED_ARCHERY;
        // }

        // Create ranged interrupt action
        Action action(PActor,
                      {
                          .category = ActionCategory::RangedStart,
                          .argument = FourCC::RangeInterrupt,
                      });

        // Add target with animation
        action.addTarget(PTarget,
                         {
                             .resolution = ActionResolution::Hit,
                             .subKind    = static_cast<ActionAnimation>(ANIMATION_RANGED_MARKSMANSHIP),
                         });

        return action;
    }

    auto MagicInterrupt(CBaseEntity* PActor, const CSpell* PSpell) -> Action
    {
        Action action(PActor,
                      {
                          .category = ActionCategory::MagicFinish,
                          .argument = PSpell->getFourCC(true),
                          .info     = 2, // 2s recast on interruptions
                      });

        action.addTarget(PActor,
                         {
                             .resolution = ActionResolution::Hit,
                             .subKind    = ActionAnimation::SkillInterrupt,
                         });

        return action;
    }

    auto WeaknessTrigger(CBaseEntity* PActor, const WeaknessType weaknessType) -> Action
    {
        ActionAnimation animationID{ 0 };
        switch (weaknessType)
        {
            case WeaknessType::RED:
                animationID = ActionAnimation::RedTrigger;
                break;
            case WeaknessType::YELLOW:
                animationID = ActionAnimation::YellowTrigger;
                break;
            case WeaknessType::BLUE:
                animationID = ActionAnimation::BlueTrigger;
                break;
            case WeaknessType::WHITE:
                animationID = ActionAnimation::WhiteTrigger;
                break;
        }

        Action action(PActor,
                      {
                          .category = ActionCategory::MobSkillFinish,
                      });

        action.addTarget(PActor,
                         {
                             .subKind = animationID,
                             .value   = 2582,
                         });

        return action;
    }

    auto RangedFinish(CBaseEntity* PActor) -> Action
    {
        return Action(PActor,
                      {
                          .category = ActionCategory::RangedFinish,
                      });
    }

    auto BasicAttack(CBaseEntity* PActor) -> Action
    {
        return Action(PActor,
                      {
                          .category = ActionCategory::BasicAttack,
                      });
    }

    auto MobSkillFinish(CBaseEntity* PActor, const CMobSkill* PSkill) -> Action
    {
        return Action(PActor,
                      {
                          .category = ActionCategory::MobSkillFinish,
                          .argument = PSkill ? PSkill->getID() : static_cast<uint32_t>(0),
                      });
    }

    auto PetSkillFinish(CBaseEntity* PActor, const CPetSkill* PSkill) -> Action
    {
        return Action(PActor,
                      {
                          .category = ActionCategory::MobSkillFinish,
                          .argument = PSkill->getID(),
                      });
    }

    auto AbilityFinish(CBaseEntity* PActor, const CAbility* PAbility) -> Action
    {
        return Action(PActor,
                      {
                          .category = ActionCategory::AbilityFinish,
                          .argument = PAbility->getID(),
                      });
    }

    auto ItemFinish(CBaseEntity* PActor, const CItemUsable* PItem) -> Action
    {
        return Action(PActor,
                      {
                          .category = ActionCategory::ItemFinish,
                          .argument = PItem->getID(),
                      });
    }

    auto MagicFinish(CBaseEntity* PActor, CSpell* PSpell) -> Action
    {
        return Action(PActor,
                      {
                          .category = ActionCategory::MagicFinish,
                          .argument = PSpell->getID(),
                      });
    }

} // namespace Actions
