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

#include "entity_render_container.h"
#include "baseentity.h"

void EntityRenderContainer::init(uint8* updatemask)
{
    updatemask_ = updatemask;
}

void EntityRenderContainer::applyNamevis(const uint8 namevis)
{
    if (namevis & VIS_ICON)
    {
        infoIcon_ = true;
    }
    if (namevis & VIS_ANIM_OVERRIDE)
    {
        animOverride_ = true;
    }
    if (namevis & VIS_SUBANIM_3BIT)
    {
        subAnim3Bit_ = true;
    }
    if (namevis & VIS_HIDE_NAME)
    {
        hiddenName_ = true;
    }
    if (namevis & VIS_NO_COLLISION)
    {
        collisionDisabled_ = true;
    }
    if (namevis & VIS_HIDE_HP_AND_NAME)
    {
        hiddenHPAndName_ = true;
    }
    if (namevis & VIS_HIDE_COMPASS)
    {
        hiddenCompass_ = true;
    }
    if (namevis & VIS_GHOST_PHASE)
    {
        halfTransparent_ = true;
    }
}

void EntityRenderContainer::applyEntityFlags(const uint32 flags)
{
    if (flags & FLAG_UNTARGETABLE)
    {
        untargetable_ = true;
    }
    if (flags & FLAG_CALL_FOR_HELP)
    {
        yellFlag_ = true;
    }
    if (flags & FLAG_HIDE_HP)
    {
        hiddenHP_ = true;
    }
    // entityFlags bit 7 (0x80) = Gender (0=female, 1=male)
    gender_ = (flags & 0x80) ? 1 : 0;

    // entityFlags bit 9 (0x200) = French Feminine article (la vs le)
    linkShellFlag_ = (flags & 0x200) != 0;

    // entityFlags bit 10 (0x400) = French Elision article (l')
    linkDeadFlag_ = (flags & 0x400) != 0;
}

void EntityRenderContainer::applyAnimationsub(const uint8 animationsub)
{
    // animationsub bit 3 (0x08) = likely German Feminine (die)
    unknownLocalization1_ = (animationsub & 0x08) != 0;

    // animationsub bit 4 (0x10) = likely German Neuter (das)
    unknownLocalization2_ = (animationsub & 0x10) != 0;
}

void EntityRenderContainer::markDirty() const
{
    if (updatemask_)
    {
        *updatemask_ |= UPDATE_COMBAT;
    }
}

void EntityRenderContainer::setHidden(const bool value)
{
    hidden_ = value;
    markDirty();
}

auto EntityRenderContainer::isHidden() const -> bool
{
    return hidden_;
}

void EntityRenderContainer::setYellFlag(const bool value)
{
    yellFlag_ = value;
    markDirty();
}

auto EntityRenderContainer::hasYellFlag() const -> bool
{
    return yellFlag_;
}

void EntityRenderContainer::setHiddenHP(const bool value)
{
    hiddenHP_ = value;
    markDirty();
}

auto EntityRenderContainer::isHPHidden() const -> bool
{
    return hiddenHP_;
}

void EntityRenderContainer::setHiddenShadow(const bool value)
{
    hiddenShadow_ = value;
    markDirty();
}

auto EntityRenderContainer::isShadowHidden() const -> bool
{
    return hiddenShadow_;
}

void EntityRenderContainer::setUntargetable(const bool value)
{
    untargetable_ = value;
    markDirty();
}

auto EntityRenderContainer::isUntargetable() const -> bool
{
    return untargetable_;
}

void EntityRenderContainer::setHiddenName(const bool value)
{
    hiddenName_ = value;
    markDirty();
}

auto EntityRenderContainer::isNameHidden() const -> bool
{
    return hiddenName_;
}

void EntityRenderContainer::setCollisionDisabled(const bool value)
{
    collisionDisabled_ = value;
    markDirty();
}

auto EntityRenderContainer::isCollisionDisabled() const -> bool
{
    return collisionDisabled_;
}

void EntityRenderContainer::setHiddenHPAndName(const bool value)
{
    hiddenHPAndName_ = value;
    markDirty();
}

auto EntityRenderContainer::isHPAndNameHidden() const -> bool
{
    return hiddenHPAndName_;
}

void EntityRenderContainer::setHiddenCompass(const bool value)
{
    hiddenCompass_ = value;
    markDirty();
}

auto EntityRenderContainer::isCompassHidden() const -> bool
{
    return hiddenCompass_;
}

void EntityRenderContainer::setHalfTransparent(const bool value)
{
    halfTransparent_ = value;
    markDirty();
}

auto EntityRenderContainer::isHalfTransparent() const -> bool
{
    return halfTransparent_;
}

void EntityRenderContainer::setInfoIcon(const bool value)
{
    infoIcon_ = value;
    markDirty();
}

auto EntityRenderContainer::hasInfoIcon() const -> bool
{
    return infoIcon_;
}

void EntityRenderContainer::setAnimOverride(const bool value)
{
    animOverride_ = value;
}

auto EntityRenderContainer::hasAnimOverride() const -> bool
{
    return animOverride_;
}

void EntityRenderContainer::setSubAnim3Bit(const bool value)
{
    subAnim3Bit_ = value;
}

auto EntityRenderContainer::hasSubAnim3Bit() const -> bool
{
    return subAnim3Bit_;
}

void EntityRenderContainer::setNamedFlag(const bool value)
{
    namedFlag_ = value;
}

auto EntityRenderContainer::hasNamedFlag() const -> bool
{
    return namedFlag_;
}

void EntityRenderContainer::setSingleFlag(const bool value)
{
    singleFlag_ = value;
}

auto EntityRenderContainer::hasSingleFlag() const -> bool
{
    return singleFlag_;
}

void EntityRenderContainer::setGender(const uint8 value)
{
    gender_ = value;
}

auto EntityRenderContainer::getGender() const -> uint8
{
    return gender_;
}

void EntityRenderContainer::setLinkShellFlag(const bool value)
{
    linkShellFlag_ = value;
}

auto EntityRenderContainer::hasLinkShellFlag() const -> bool
{
    return linkShellFlag_;
}

void EntityRenderContainer::setLinkDeadFlag(const bool value)
{
    linkDeadFlag_ = value;
}

auto EntityRenderContainer::hasLinkDeadFlag() const -> bool
{
    return linkDeadFlag_;
}

void EntityRenderContainer::setUnknownLocalization1(const bool value)
{
    unknownLocalization1_ = value;
}

auto EntityRenderContainer::hasUnknownLocalization1() const -> bool
{
    return unknownLocalization1_;
}

void EntityRenderContainer::setUnknownLocalization2(const bool value)
{
    unknownLocalization2_ = value;
}

auto EntityRenderContainer::hasUnknownLocalization2() const -> bool
{
    return unknownLocalization2_;
}
