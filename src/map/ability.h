/*
===========================================================================

  Copyright (c) 2010-2015 Darkstar Dev Teams

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

#include "data/enums/ability_add_type.h"
#include "data/enums/job_ability.h"
#include "data/enums/recast.h"
#include "entities/battle_entity.h"
#include "enums/action/animation.h"
#include "status_effect.h"

enum class ActionCategory : uint8_t;

struct Charge_t
{
    uint16          ID;         // recastId
    xi::Job         job;        // job
    uint8           level;      // level
    uint8           maxCharges; // maximum number of stored charges
    timer::duration chargeTime; // time required to restore one charge
    uint16          merit;
};

/************************************************************************
 *                                                                       *
 *                                                                       *
 *                                                                       *
 ************************************************************************/

class CAbility
{
public:
    CAbility(xi::JobAbility id);

    auto isPetAbility() const -> bool;
    auto isAoE() const -> bool;
    auto isConal() -> bool;

    auto getID() const -> xi::JobAbility;
    auto getMobSkillID() const -> uint16;
    auto getJob() -> xi::Job;
    auto getLevel() const -> uint8;
    auto getAnimationID() const -> ActionAnimation;
    auto getAnimationTime() -> timer::duration;
    auto getCastTime() -> timer::duration;
    auto getRange() const -> float;
    auto getAOE() const -> uint8;
    auto getRadius() const -> uint8;
    auto getValidTarget() const -> uint16;
    auto getAddType() const -> xi::AbilityAddType;
    auto hasAddType(xi::AbilityAddType addType) const -> bool;
    auto getMessage() const -> MsgBasic;
    auto getRecastTime() const -> timer::duration;
    auto getRecastId() const -> xi::Recast;
    auto getCE() const -> int32;
    auto getVE() const -> int32;
    auto getMeritModID() const -> uint16;
    auto getActionType() const -> ActionCategory;
    auto getPostActionEffectCleanup() -> xi::StatusEffect;

    void setID(xi::JobAbility id);
    void setJob(xi::Job Job);
    void setLevel(uint8 level);
    void setAnimationID(uint16 animationID);
    void setAnimationTime(timer::duration time);
    void setCastTime(timer::duration time);
    void setRange(float range);
    void setAOE(uint8 aoe);
    void setRadius(uint8 radius);
    void setValidTarget(uint16 validTarget);
    void setAddType(xi::AbilityAddType addType);
    void setMessage(MsgBasic message);
    void setRecastTime(timer::duration recastTime);
    void setRecastId(xi::Recast recastId);
    void setCE(int32 CE);
    void setVE(int32 VE);
    void setMeritModID(uint16 value);
    void setActionType(ActionCategory type);
    void setPostActionEffectCleanup(xi::StatusEffect effectToCleanup);

    auto getName() -> const std::string&;
    void setName(const std::string& name);

private:
    xi::JobAbility     m_ID;
    xi::Job            m_Job;
    uint8              m_level;
    uint16             m_animationID;
    timer::duration    m_animationTime{};
    timer::duration    m_castTime{};
    float              m_range;
    uint8              m_aoe;
    uint8              m_radius{ 0 };
    uint16             m_validTarget;
    xi::AbilityAddType m_addType{};
    MsgBasic           m_message;
    timer::duration    m_recastTime{};
    xi::Recast         m_recastId;
    int32              m_CE;
    int32              m_VE;
    uint16             m_meritModID;
    std::string        m_name;
    ActionCategory     m_actionType{};
    xi::StatusEffect   m_cleanupEffect{};
};

/************************************************************************
 *                                                                       *
 *  Namespace to work with abilities                                     *
 *                                                                       *
 ************************************************************************/

namespace ability
{

void LoadAbilitiesList();

auto GetAbility(xi::JobAbility AbilityID) -> CAbility*;

auto GetTwoHourAbility(xi::Job JobID) -> CAbility*;
auto CanLearnAbility(CBattleEntity* PUser, uint16 AbilityID) -> bool;
auto GetCharge(CBattleEntity* PUser, uint16 chargeID) -> Charge_t*;

auto GetAbilities(xi::Job JobID) -> std::vector<CAbility*>;

}; // namespace ability
