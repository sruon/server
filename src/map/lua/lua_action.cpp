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

#include "lua_action.h"

#include "action/action.h"

CLuaAction::CLuaAction(Action* Action)
: m_PLuaAction(Action)
{
    if (Action == nullptr)
    {
        ShowError("CLuaAction created with nullptr instead of valid Action*!");
    }
}

void CLuaAction::ID(uint32 actionTargetID, uint32 newActionTargetID)
{
    m_PLuaAction->target(actionTargetID).setTargetId(newActionTargetID);
}

// Get the first (primary) target's long ID, if available.
uint32 CLuaAction::getPrimaryTargetID() const
{
    if (m_PLuaAction->targetCount() > 0)
    {
        return m_PLuaAction->target().targetId();
    }

    return 0;
}

void CLuaAction::setRecast(uint16 recast)
{
    m_PLuaAction->setRecast(std::chrono::seconds(recast));
}

uint16 CLuaAction::getRecast()
{
    return static_cast<uint16>(m_PLuaAction->info());
}

void CLuaAction::actionID(uint16 actionid)
{
    m_PLuaAction->setArgument(static_cast<uint32_t>(actionid));
}

uint16 CLuaAction::getParam(uint32 actionTargetID)
{
    return static_cast<uint16>(m_PLuaAction->target(actionTargetID).result(0).value());
}

void CLuaAction::param(uint32 actionTargetID, int32 param)
{
    m_PLuaAction->target(actionTargetID).result(0).setValue(param);
}

void CLuaAction::messageID(uint32 actionTargetID, uint16 messageID)
{
    m_PLuaAction->target(actionTargetID).result(0).setMessage(static_cast<MSGBASIC_ID>(messageID));
}

std::optional<uint16> CLuaAction::getMsg(uint32 actionTargetID)
{
    return static_cast<uint16>(m_PLuaAction->target(actionTargetID).result(0).message());
}

auto CLuaAction::getAnimation(const uint32 actionTargetID) const -> ActionSubKind
{
    return m_PLuaAction->target(actionTargetID).result(0).subKind();
}

void CLuaAction::setAnimation(const uint32 actionTargetID, const ActionSubKind animation) const
{
    m_PLuaAction->target(actionTargetID).result(0).setSubKind(animation);
}

auto CLuaAction::getCategory() -> uint8
{
    return static_cast<uint8>(m_PLuaAction->category());
}

void CLuaAction::setCategory(uint8 category)
{
    m_PLuaAction->setCategory(static_cast<ActionCategory>(category));
}

void CLuaAction::speceffect(uint32 actionTargetID, uint8 speceffect)
{
    auto& result = m_PLuaAction->target(actionTargetID).result(0);
    // TODO: Hella wrong (same as old implementation)
    result.setInfo(static_cast<ActionInfo>(speceffect & 0x1F));
    result.setDistortion(static_cast<HitDistortion>(speceffect >> 5 & 0x03));
}

void CLuaAction::reaction(uint32 actionTargetID, uint8 reaction)
{
    m_PLuaAction->target(actionTargetID).result(0).addReaction({
        .type = static_cast<ActionReactKind>(reaction),
    });
}

void CLuaAction::modifier(uint32 actionTargetID, uint8 modifier)
{
    m_PLuaAction->target(actionTargetID).result(0).setModifier(static_cast<ActionModifier>(modifier));
}

void CLuaAction::additionalEffect(uint32 actionTargetID, uint16 additionalEffect)
{
    auto& result = m_PLuaAction->target(actionTargetID).result(0);

    ProcSpec procSpec;
    if (result.hasProc())
    {
        procSpec = result.proc();
    }
    procSpec.type = static_cast<ActionProcAddEffect>(additionalEffect);
    result.addProc(procSpec);
}

void CLuaAction::addEffectParam(uint32 actionTargetID, int32 addEffectParam)
{
    auto& result = m_PLuaAction->target(actionTargetID).result(0);

    if (result.hasProc())
    {
        auto procSpec  = result.proc();
        procSpec.value = addEffectParam;
        result.addProc(procSpec);
    }
    else
    {
        ShowWarning("Setting action proc_value on action without proc_kind");
    }
}

void CLuaAction::addEffectMessage(uint32 actionTargetID, uint16 addEffectMessage)
{
    auto& result = m_PLuaAction->target(actionTargetID).result(0);

    if (result.hasProc())
    {
        auto procSpec    = result.proc();
        procSpec.message = static_cast<MSGBASIC_ID>(addEffectMessage);
        result.addProc(procSpec);
    }
    else
    {
        ShowWarning("Setting action proc_message on action without proc_kind");
    }
}

bool CLuaAction::addAdditionalTarget(uint32 actionTargetID)
{
    for (size_t i = 0; i < m_PLuaAction->targetCount(); ++i)
    {
        if (m_PLuaAction->target(i).targetId() == actionTargetID)
        {
            return false;
        }
    }

    m_PLuaAction->addTarget(actionTargetID, {});
    return true;
}

//==========================================================//

void CLuaAction::Register()
{
    SOL_USERTYPE("CAction", CLuaAction);
    SOL_REGISTER("ID", CLuaAction::ID);
    SOL_REGISTER("getPrimaryTargetID", CLuaAction::getPrimaryTargetID);
    SOL_REGISTER("getRecast", CLuaAction::getRecast);
    SOL_REGISTER("setRecast", CLuaAction::setRecast);
    SOL_REGISTER("actionID", CLuaAction::actionID);
    SOL_REGISTER("getParam", CLuaAction::getParam);
    SOL_REGISTER("param", CLuaAction::param);
    SOL_REGISTER("messageID", CLuaAction::messageID);
    SOL_REGISTER("getMsg", CLuaAction::getMsg);
    SOL_REGISTER("getAnimation", CLuaAction::getAnimation);
    SOL_REGISTER("setAnimation", CLuaAction::setAnimation);
    SOL_REGISTER("getCategory", CLuaAction::getCategory);
    SOL_REGISTER("setCategory", CLuaAction::setCategory);
    SOL_REGISTER("speceffect", CLuaAction::speceffect);
    SOL_REGISTER("reaction", CLuaAction::reaction);
    SOL_REGISTER("modifier", CLuaAction::modifier);
    SOL_REGISTER("additionalEffect", CLuaAction::additionalEffect);
    SOL_REGISTER("addEffectParam", CLuaAction::addEffectParam);
    SOL_REGISTER("addEffectMessage", CLuaAction::addEffectMessage);
    SOL_REGISTER("addAdditionalTarget", CLuaAction::addAdditionalTarget);
};

std::ostream& operator<<(std::ostream& os, const CLuaAction& action)
{
    std::string id = action.m_PLuaAction ? std::to_string(action.m_PLuaAction->actorId()) : "nullptr";
    return os << "CLuaAction(" << id << ")";
}

//==========================================================//
