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

    // Extract German localization flags from animationsub bits 3-4
    void applyAnimationsub(uint8 animationsub);

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

    // Flags3.unknown_3_1. Uses alternate animation values (e.g., different chair sitting).
    // Seen on Trusts in retail.
    void setAnimOverride(bool value);
    bool hasAnimOverride() const;

    // Flags3.unknown_3_2. SubAnimation uses 3 bits instead of 2 by default.
    // Activated during entity spawn events. Seen on Trusts and Ethereal Junctions.
    void setSubAnim3Bit(bool value);
    bool hasSubAnim3Bit() const;

    // Flags2.NamedFlag. Adds "the" prefix to mob/NPC names.
    void setNamedFlag(bool value);
    bool hasNamedFlag() const;

    // Flags2.SingleFlag. Purpose unclear - used by some mobs/NPCs.
    void setSingleFlag(bool value);
    bool hasSingleFlag() const;

    // Flags1.Gender. entityFlags bit 7 (0x80) = Gender value (0=female, 1=male).
    void setGender(uint8 value);
    uint8 getGender() const;

    // French localization article flags.
    // These control which definite article is used when displaying entity names in French:
    //   LinkShellFlag=0, LinkDeadFlag=0 → "le" (masculine, consonant)
    //   LinkShellFlag=0, LinkDeadFlag=1 → "l'" (masculine, vowel/h muet - elision)
    //   LinkShellFlag=1, LinkDeadFlag=0 → "la" (feminine, consonant)
    //   LinkShellFlag=1, LinkDeadFlag=1 → "l'" (feminine, vowel/h muet - elision)

    // Flags1.LinkShellFlag. entityFlags bit 9 (0x200) = French Feminine article (la vs le).
    void setLinkShellFlag(bool value);
    bool hasLinkShellFlag() const;

    // Flags1.LinkDeadFlag. entityFlags bit 10 (0x400) = French Elision article (l').
    void setLinkDeadFlag(bool value);
    bool hasLinkDeadFlag() const;

    // German localization article flags (very likely).
    // These are stored in animationsub bits 3-4 and likely control German definite articles:
    //   unknownLocalization1=0, unknownLocalization2=0 → "der" (masculine)
    //   unknownLocalization1=1, unknownLocalization2=0 → "die" (feminine)
    //   unknownLocalization1=0, unknownLocalization2=1 → "das" (neuter)
    //   unknownLocalization1=1, unknownLocalization2=1 → never used

    // Flags3.unknown_2_3. animationsub bit 3 (0x08) = likely German Feminine (die).
    void setUnknownLocalization1(bool value);
    bool hasUnknownLocalization1() const;

    // Flags3.unknown_2_4. animationsub bit 4 (0x10) = likely German Neuter (das).
    void setUnknownLocalization2(bool value);
    bool hasUnknownLocalization2() const;

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
    bool   animOverride_      = false;
    bool   subAnim3Bit_       = false;
    bool   namedFlag_         = false;
    bool   singleFlag_        = false;
    bool   linkShellFlag_         = false;
    bool   linkDeadFlag_          = false;
    bool   unknownLocalization1_  = false;
    bool   unknownLocalization2_  = false;
    uint8  gender_                = 1;
};
