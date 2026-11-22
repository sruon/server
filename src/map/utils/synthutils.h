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
#include "data/enums/content.h"
#include "data/enums/key_item.h"

#include <array>
#include <string>
#include <utility>
#include <vector>

/************************************************************************
 *                                                                       *
 *  Methods required for the synthesis to work                           *
 *                                                                       *
 ************************************************************************/

class CCharEntity;
struct SynthOffer;

namespace synthutils
{

enum SYNTHESIS_ELEMENT
{
    ELEMENT_FIRE      = 0,
    ELEMENT_ICE       = 1,
    ELEMENT_WIND      = 2,
    ELEMENT_EARTH     = 3,
    ELEMENT_LIGHTNING = 4,
    ELEMENT_WATER     = 5,
    ELEMENT_LIGHT     = 6,
    ELEMENT_DARK      = 7,
};

struct SynthRecipe
{
    uint32                                  ID{};
    bool                                    Desynth{};
    xi::KeyItem                             RequiredKeyItem{};
    std::array<uint8, 8>                    Skills{}; // woodworking to cooking
    uint16                                  Crystal{};
    std::array<uint16, 8>                   Ingredients{}; // sorted, 0 in empty slots
    std::array<std::pair<uint16, uint8>, 4> Results{};     // item and quantity, normal quality then high quality 1 to 3
    std::string                             Name;
    xi::Content                             Content{};
};

void LoadSynthRecipes();

// Recipes the recipe menu of a craft (1 woodworking to 8 cooking) lists, in menu order.
auto getMenuRecipes(uint16 craft, uint8 minLevel, uint8 maxLevel) -> std::vector<const SynthRecipe*>;
void startSynth(CCharEntity* PChar, const SynthOffer& offer);
void sendSynthDone(CCharEntity* PChar);
void doSynthCriticalFail(CCharEntity* PChar);

}; // namespace synthutils
