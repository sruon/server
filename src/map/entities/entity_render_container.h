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

#include "common/cbasetypes.h"

class EntityRenderContainer
{
public:
    void init(uint8* updatemask);

    // Convert legacy namevis flags to render flags (temporary migration helper)
    void applyNamevis(uint8 namevis);

    // Convert legacy entityFlags to render flags (temporary migration helper)
    void applyEntityFlags(uint32 flags);

    // Flags1.HideFlag. Used by Worms, Antlions for ??? display.
    void setHidden(bool value);
    bool isHidden() const;

    // Flags1.YellFlag. Mobs with call for help status.
    void setYellFlag(bool value);
    bool hasYellFlag() const;

    // Flags1.PlayOnelineFlag. HP bar is hidden when targeted. Dark Ixion.
    void setHiddenHP(bool value);
    bool isHPHidden() const;

    // Flags2.ShadowFlag. Flag set if the entities shadow should be hidden.
    void setHiddenShadow(bool value);
    bool isShadowHidden() const;

    // Flags1.TargetOffFlag. Entity cannot be targeted.
    void setUntargetable(bool value);
    bool isUntargetable() const;

    // Flags3.unknown_3_3. Name visibility via hideName() Lua binding.
    void setHiddenName(bool value);
    bool isNameHidden() const;

    // Flags3.unknown_3_4. Causes the entity to become non-blocking.
    void setCollisionDisabled(bool value);
    bool isCollisionDisabled() const;

    // Flags3.unknown_3_5. Cause the entities health bar to be hidden and the name above their head to not be rendered.
    void setHiddenHPAndName(bool value);
    bool isHPAndNameHidden() const;

    // Flags3.unknown_3_6. Entity will not be drawn on the clients compass.
    void setHiddenCompass(bool value);
    bool isCompassHidden() const;

    // Flags3.unknown_3_7. Entity becomes half-transparent.
    void setHalfTransparent(bool value);
    bool isHalfTransparent() const;

    // Flags1.InfoFlag. Shows (I) icon next to name.
    void setInfoIcon(bool value);
    bool hasInfoIcon() const;

    // Flags2.NamedFlag. Adds "the" prefix to mob/NPC names.
    void setNamedFlag(bool value);
    bool hasNamedFlag() const;

    // Flags2.SingleFlag. Purpose unclear - used by some mobs/NPCs.
    void setSingleFlag(bool value);
    bool hasSingleFlag() const;

private:
    void markDirty() const;

    uint8* updatemask_        = nullptr;
    bool   hidden_            = false;
    bool   yellFlag_          = false;
    bool   hiddenHP_          = false;
    bool   hiddenShadow_      = false;
    bool   untargetable_      = false;
    bool   hiddenName_        = false;
    bool   collisionDisabled_ = false;
    bool   hiddenHPAndName_   = false;
    bool   hiddenCompass_     = false;
    bool   halfTransparent_   = false;
    bool   infoIcon_          = false;
    bool   namedFlag_         = false;
    bool   singleFlag_        = false;
};
