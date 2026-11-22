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

#include "ability.h"

#include "data/datasets/abilities/dataset.h"
#include "data/enums/recast.h"

#include "enums/action/category.h"
#include "enums/msg_basic.h"
#include "lua/luautils.h"
#include "utils/dataset_loader.h"

#include <algorithm>
#include <array>
#include <map>
#include <stdexcept>
#include <utility>

CAbility::CAbility(xi::JobAbility id)
: m_ID(id)
, m_Job(xi::Job::NONE)
, m_level(0)
, m_animationID(0)
, m_range(0)
, m_aoe(0)
, m_validTarget(0)
, m_message(MsgBasic::None)
, m_recastTime(0s)
, m_recastId(xi::Recast::Special)
, m_CE(0)
, m_VE(0)
, m_meritModID(0)
{
}

auto CAbility::isPetAbility() const -> bool
{
    return ((getID() >= xi::JobAbility::ConcentricPulse && getID() <= xi::JobAbility::RadialArcana) || getID() >= xi::JobAbility::HealingRuby);
}

auto CAbility::isAoE() const -> bool
{
    return m_aoe == 1;
}

auto CAbility::isConal() -> bool
{
    // no abilities are conal?
    return false;
}

void CAbility::setID(xi::JobAbility id)
{
    m_ID = id;
}

auto CAbility::getID() const -> xi::JobAbility
{
    return m_ID;
}

void CAbility::setJob(xi::Job Job)
{
    m_Job = Job;
}

void CAbility::setMeritModID(uint16 value)
{
    m_meritModID = value;
}

void CAbility::setActionType(const ActionCategory type)
{
    m_actionType = type;
}

void CAbility::setPostActionEffectCleanup(xi::StatusEffect effectToCleanup)
{
    m_cleanupEffect = effectToCleanup;
}

auto CAbility::getJob() -> xi::Job
{
    return m_Job;
}

void CAbility::setLevel(uint8 level)
{
    m_level = level;
}

auto CAbility::getLevel() const -> uint8
{
    return m_level;
}

void CAbility::setRange(float range)
{
    m_range = range;
}

auto CAbility::getRange() const -> float
{
    return m_range;
}

void CAbility::setAOE(uint8 aoe)
{
    m_aoe = aoe;
}

auto CAbility::getAOE() const -> uint8
{
    return m_aoe;
}

void CAbility::setRadius(uint8 radius)
{
    m_radius = radius;
}

auto CAbility::getRadius() const -> uint8
{
    return m_radius;
}

void CAbility::setAnimationID(uint16 animationID)
{
    m_animationID = animationID;
}

void CAbility::setAnimationTime(timer::duration time)
{
    m_animationTime = time;
}

void CAbility::setCastTime(timer::duration time)
{
    m_castTime = time;
}

auto CAbility::getAnimationID() const -> ActionAnimation
{
    return static_cast<ActionAnimation>(m_animationID);
}

auto CAbility::getAnimationTime() -> timer::duration
{
    return m_animationTime;
}

auto CAbility::getCastTime() -> timer::duration
{
    return m_castTime;
}

void CAbility::setRecastTime(timer::duration recastTime)
{
    m_recastTime = std::chrono::floor<std::chrono::milliseconds>(recastTime * settings::get<float>("map.ABILITY_RECAST_MULTIPLIER"));
}

auto CAbility::getRecastTime() const -> timer::duration
{
    return m_recastTime;
}

auto CAbility::getMeritModID() const -> uint16
{
    return m_meritModID;
}

auto CAbility::getActionType() const -> ActionCategory
{
    return m_actionType;
}

auto CAbility::getPostActionEffectCleanup() -> xi::StatusEffect
{
    return m_cleanupEffect;
}

void CAbility::setValidTarget(uint16 validTarget)
{
    m_validTarget = validTarget;
}

auto CAbility::getValidTarget() const -> uint16
{
    return m_validTarget;
}

auto CAbility::getAddType() const -> xi::AbilityAddType
{
    return m_addType;
}

auto CAbility::hasAddType(const xi::AbilityAddType addType) const -> bool
{
    return (m_addType & addType) != xi::AbilityAddType::None;
}

void CAbility::setAddType(xi::AbilityAddType addType)
{
    m_addType = addType;
}

auto CAbility::getName() -> const std::string&
{
    return m_name;
}

void CAbility::setName(const std::string& name)
{
    m_name = name;
}

auto CAbility::getRecastId() const -> xi::Recast
{
    return m_recastId;
}

void CAbility::setRecastId(const xi::Recast recastId)
{
    m_recastId = recastId;
}

void CAbility::setCE(int32 CE)
{
    m_CE = CE;
}

auto CAbility::getCE() const -> int32
{
    return m_CE;
}

void CAbility::setVE(int32 VE)
{
    m_VE = VE;
}

auto CAbility::getVE() const -> int32
{
    return m_VE;
}

/************************************************************************
 *                                                                       *
 *  Get/Set message abilities                                            *
 *                                                                       *
 ************************************************************************/

auto CAbility::getMessage() const -> MsgBasic
{
    return m_message;
}

void CAbility::setMessage(MsgBasic message)
{
    m_message = message;
}

/************************************************************************
 *                                                                       *
 *  Namespace implementation for working with abilities                  *
 *                                                                       *
 ************************************************************************/

namespace ability
{

namespace
{

std::map<xi::JobAbility, std::unique_ptr<CAbility>> PAbilityList;    // Complete Abilities List
std::map<xi::Job, std::vector<CAbility*>>           PAbilitiesByJob; // Abilities by Job
std::vector<std::unique_ptr<Charge_t>>              PChargesList;    // Abilities with charges

auto toTargetType(const xi::ActionTarget targets, const std::string_view name) -> uint16
{
    constexpr std::array mapping{
        std::pair{ xi::ActionTarget::Self, TARGET_SELF },
        std::pair{ xi::ActionTarget::Pet, TARGET_PET },
        std::pair{ xi::ActionTarget::Party, TARGET_PLAYER_PARTY },
        std::pair{ xi::ActionTarget::Alliance, TARGET_PLAYER_ALLIANCE },
        std::pair{ xi::ActionTarget::Friendly, TARGET_PLAYER },
        std::pair{ xi::ActionTarget::Hostile, TARGET_ENEMY },
        std::pair{ xi::ActionTarget::Dead, TARGET_PLAYER_DEAD },
    };

    uint16 result  = 0;
    auto   pending = targets;
    for (const auto& [target, type] : mapping)
    {
        if ((targets & target) != xi::ActionTarget::None)
        {
            result |= type;
            pending = pending & ~target;
        }
    }

    if (pending != xi::ActionTarget::None)
    {
        throw std::runtime_error(fmt::format("{} has a target abilities do not support", name));
    }

    return result;
}

} // namespace

/************************************************************************
 *                                                                       *
 *  Load Abilities from data/abilities.yaml                              *
 *                                                                       *
 ************************************************************************/

void LoadAbilitiesList()
{
    auto abilities = xi::data::loadDataset<xi::data::datasets::abilities::Dataset>();

    // Abilities by job, lowest level first
    std::ranges::stable_sort(abilities, {}, &xi::data::AbilityData::Level);
    std::ranges::stable_sort(abilities, {}, &xi::data::AbilityData::Job);

    for (const auto& entry : abilities)
    {
        if (entry.Charges)
        {
            for (const auto& tier : entry.Charges->Tiers)
            {
                PChargesList.emplace_back(std::make_unique<Charge_t>(Charge_t{
                    .ID         = entry.Charges->RecastId,
                    .job        = entry.Job,
                    .level      = tier.Level,
                    .maxCharges = tier.MaxCharges,
                    .chargeTime = std::chrono::seconds(tier.ChargeTime),
                    .merit      = std::to_underlying(tier.Merit),
                }));
            }
        }

        if (!luautils::IsContentEnabled(entry.Content))
        {
            continue;
        }

        auto PAbility = std::make_unique<CAbility>(entry.Id);
        PAbility->setName(entry.Name);
        PAbility->setJob(entry.Job);
        PAbility->setLevel(entry.Level);
        PAbility->setValidTarget(toTargetType(entry.Target, entry.Name));
        PAbility->setRecastTime(std::chrono::seconds(entry.Recast));
        PAbility->setMessage(static_cast<MsgBasic>(entry.Message));
        PAbility->setAnimationID(entry.Animation);
        PAbility->setCastTime(std::chrono::milliseconds(entry.CastTime));
        PAbility->setActionType(static_cast<ActionCategory>(std::to_underlying(entry.Category)));
        PAbility->setAnimationTime(std::chrono::milliseconds(entry.AnimationTime));
        PAbility->setRange(entry.Range);
        PAbility->setRecastId(static_cast<xi::Recast>(entry.RecastId));
        PAbility->setCE(entry.CE);
        PAbility->setVE(entry.VE);
        PAbility->setMeritModID(std::to_underlying(entry.Merit));
        PAbility->setAddType(entry.AddType);
        PAbility->setAOE(static_cast<uint8>(entry.Aoe));
        PAbility->setRadius(entry.Radius);

        auto filename = fmt::format("./scripts/actions/abilities/{}.lua", PAbility->getName());
        if (PAbility->isPetAbility())
        {
            filename = fmt::format("./scripts/actions/abilities/pets/{}.lua", PAbility->getName());
        }
        luautils::LoadLuaObjectFromFile(filename);

        PAbilitiesByJob[PAbility->getJob()].emplace_back(PAbility.get());
        PAbilityList[entry.Id] = std::move(PAbility);
    }
}

/************************************************************************
 *                                                                       *
 *  Get Ability By ID                                                    *
 *                                                                       *
 ************************************************************************/

auto GetAbility(xi::JobAbility AbilityID) -> CAbility*
{
    if (auto itr = PAbilityList.find(AbilityID); itr != PAbilityList.end())
    {
        return itr->second.get();
    }
    ShowDebug("Unable to look up ability %d", std::to_underlying(AbilityID));
    return nullptr;
}

/************************************************************************
 *                                                                       *
 *  Get the initial (SP) ability of a job                                *
 *                                                                       *
 ************************************************************************/

auto GetTwoHourAbility(xi::Job JobID) -> CAbility*
{
    if (static_cast<uint8>(JobID) >= static_cast<uint8>(xi::Job::WAR) || static_cast<uint8>(JobID) <= MAX_JOBTYPE)
    {
        switch (JobID)
        {
            case xi::Job::WAR:
                return GetAbility(xi::JobAbility::MightyStrikes);
                break;
            case xi::Job::MNK:
                return GetAbility(xi::JobAbility::HundredFists);
                break;
            case xi::Job::WHM:
                return GetAbility(xi::JobAbility::Benediction);
                break;
            case xi::Job::BLM:
                return GetAbility(xi::JobAbility::Manafont);
                break;
            case xi::Job::RDM:
                return GetAbility(xi::JobAbility::Chainspell);
                break;
            case xi::Job::THF:
                return GetAbility(xi::JobAbility::PerfectDodge);
                break;
            case xi::Job::PLD:
                return GetAbility(xi::JobAbility::Invincible);
                break;
            case xi::Job::DRK:
                return GetAbility(xi::JobAbility::BloodWeapon);
                break;
            case xi::Job::BST:
                return GetAbility(xi::JobAbility::Familiar);
                break;
            case xi::Job::BRD:
                return GetAbility(xi::JobAbility::SoulVoice);
                break;
            case xi::Job::RNG:
                return GetAbility(xi::JobAbility::EagleEyeShot);
                break;
            case xi::Job::SAM:
                return GetAbility(xi::JobAbility::MeikyoShisui);
                break;
            case xi::Job::NIN:
                return GetAbility(xi::JobAbility::MijinGakure);
                break;
            case xi::Job::DRG:
                return GetAbility(xi::JobAbility::SpiritSurge);
                break;
            case xi::Job::SMN:
                return GetAbility(xi::JobAbility::AstralFlow);
                break;
            case xi::Job::BLU:
                return GetAbility(xi::JobAbility::AzureLore);
                break;
            case xi::Job::COR:
                return GetAbility(xi::JobAbility::WildCard);
                break;
            case xi::Job::PUP:
                return GetAbility(xi::JobAbility::Overdrive);
                break;
            case xi::Job::DNC:
                return GetAbility(xi::JobAbility::Trance);
                break;
            case xi::Job::SCH:
                return GetAbility(xi::JobAbility::TabulaRasa);
                break;
            case xi::Job::GEO:
                return GetAbility(xi::JobAbility::Bolster);
                break;
            case xi::Job::RUN:
                return GetAbility(xi::JobAbility::ElementalSforzo);
                break;
            default:
                break;
        }
    }

    ShowWarning("Attempt to get two hour ability with invalid JOBTYPE %d.", static_cast<uint8>(JobID));
    return nullptr;
}

auto CanLearnAbility(CBattleEntity* PUser, uint16 AbilityID) -> bool
{
    auto* PAbility = GetAbility(static_cast<xi::JobAbility>(AbilityID));
    if (PAbility)
    {
        const auto Job    = PAbility->getJob();
        uint8      JobLvl = PAbility->getLevel();

        return ((PUser->GetMJob() == Job && PUser->GetMLevel() >= JobLvl) || (PUser->GetSJob() == Job && PUser->GetSLevel() >= JobLvl));
    }
    return false;
}

/************************************************************************
 *                                                                       *
 *  Get Abilities By JobID                                               *
 *                                                                       *
 ************************************************************************/

auto GetAbilities(xi::Job JobID) -> std::vector<CAbility*>
{
    return PAbilitiesByJob[JobID];
}

auto GetCharge(CBattleEntity* PUser, uint16 chargeID) -> Charge_t*
{
    Charge_t* charge = nullptr;
    for (auto& PCharge : PChargesList)
    {
        if (PCharge->ID == chargeID)
        {
            if (PUser->GetMJob() == PCharge->job)
            {
                if (PUser->GetMLevel() >= PCharge->level)
                {
                    charge = PCharge.get();
                }
                else
                {
                    break;
                }
            }
            else if (PUser->GetSJob() == PCharge->job)
            {
                if (PUser->GetSLevel() >= PCharge->level)
                {
                    charge = PCharge.get();
                }
                else
                {
                    break;
                }
            }
        }
    }
    return charge;
}

}; // namespace ability
