/*
===========================================================================

  Copyright (c) 2026 LandSandBoat Dev Teams

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

#include <array>

// Active synth: recipe, skill requirements, result data.

enum CRAFT_TYPE
{
    CRAFT_SYNTHESIS         = 0,
    CRAFT_DESYNTHESIS       = 1,
    CRAFT_SYNTHESIS_NO_LOSS = 2,
};

struct CCraftState
{
    struct Ingredient
    {
        uint16 itemId{};
        uint8  invSlot{ 0xFF };
    };

    struct Result
    {
        uint16 itemId{};
        uint8  qty{};
    };

    // Offer from 0x096 combine_ask.
    uint16                    crystalItemId{};
    uint8                     crystalInvSlot{ 0xFF };
    std::array<Ingredient, 8> ingredients{};

    // Recipe id + results indexed by synth-result code: [1] success,
    // [2..4] HQ1..HQ3. [0] unused.
    uint16                recipeId{};
    std::array<Result, 5> results{};

    // Per-skill requirement, indexed by (skillID - SKILL_WOODWORKING).
    std::array<uint8, 13> skillRequired{};

    uint8 element{};
    uint8 craftMode{};
    uint8 dominantSkill{};
    uint8 result{};

    auto isSynthing() const -> bool
    {
        return crystalItemId != 0;
    }

    void clean()
    {
        *this = CCraftState{};
    }
};
