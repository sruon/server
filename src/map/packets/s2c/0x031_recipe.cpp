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

#include "0x031_recipe.h"

#include "common/xirand.h"
#include "utils/synthutils.h"

#include <array>
#include <map>

namespace
{

void fillRecipeDetails(const synthutils::SynthRecipe& recipe, GP_SERV_COMMAND_RECIPE_TYPE1_3& details, const uint16 skillID)
{
    std::array<uint16, 3> subcraftIDs{};
    size_t                subidx = 0;

    // Each craft can have up to 3 subcrafts, packed in craft order.
    for (uint16 craft = 1; craft <= recipe.Skills.size() && subidx < subcraftIDs.size(); ++craft)
    {
        if (craft != skillID && recipe.Skills[craft - 1] > 0)
        {
            subcraftIDs[subidx] = craft;
            subidx++;
        }
    }

    details.productitem   = recipe.Results[0].first;
    details.need_skill_1  = subcraftIDs[0];
    details.need_skill_2  = subcraftIDs[1];
    details.need_skill_3  = subcraftIDs[2];
    details.need_item     = recipe.Crystal;
    details.need_key_item = static_cast<uint16>(recipe.RequiredKeyItem);

    std::map<uint16, uint16> ingredients;
    for (const auto ingredient : recipe.Ingredients)
    {
        if (ingredient != 0)
        {
            ++ingredients[ingredient];
        }
    }

    auto idx = 0;
    for (const auto& [itemId, itemCount] : ingredients)
    {
        details.itemnum[idx]   = itemId;
        details.itemcount[idx] = itemCount;
        idx++;
    }
}

} // namespace

GP_SERV_COMMAND_RECIPE::GP_SERV_COMMAND_RECIPE(GP_SERV_COMMAND_RECIPE_TYPE type, const uint16 skillID, const uint16 skillLevel, const uint8 skillRank, const uint16 offset)
{
    auto& packet = this->data();

    switch (type)
    {
        case GP_SERV_COMMAND_RECIPE_TYPE::RecipeDetail1:
        {
            // We don't get a skillRank from the client request, figure out the rank range based off level.
            // Retail observed to send a random recipe up to +10 but unclear if it's strictly limited to current rank and if it can send back recipes below your current skill.
            const uint8 calculatedRank = skillLevel / 10;
            const uint8 minSkill       = calculatedRank * 10;
            const uint8 maxSkill       = (calculatedRank + 1) * 10;

            if (const auto recipes = synthutils::getMenuRecipes(skillID, minSkill, maxSkill); !recipes.empty())
            {
                fillRecipeDetails(*xirand::GetRandomElement(recipes), packet.Details, skillID);
            }

            packet.Details.Type = GP_SERV_COMMAND_RECIPE_TYPE::RecipeDetail1;
            break;
        }
        case GP_SERV_COMMAND_RECIPE_TYPE::RecipeDetail2:
        {
            uint8 minSkill = skillRank * 10;
            uint8 maxSkill = (skillRank + 1) * 10;

            if (skillLevel < maxSkill)
            {
                maxSkill = skillLevel;
            }

            if (const auto recipes = synthutils::getMenuRecipes(skillID, minSkill, maxSkill); offset < recipes.size())
            {
                fillRecipeDetails(*recipes[offset], packet.Details, skillID);
            }

            packet.Details.Type = GP_SERV_COMMAND_RECIPE_TYPE::RecipeDetail2;
            break;
        }
        case GP_SERV_COMMAND_RECIPE_TYPE::RecipeList:
        {
            const uint8 minSkill = skillRank * 10;
            uint8       maxSkill = (skillRank + 1) * 10;
            uint8       idx      = 0;

            if (skillLevel < maxSkill)
            {
                maxSkill = skillLevel;
            }

            const auto recipes = synthutils::getMenuRecipes(skillID, minSkill, maxSkill);
            for (size_t index = offset; index < recipes.size(); ++index)
            {
                if (idx == 16)
                {
                    // The 17th result of a query is not displayed in the menu, but instead is used to signal
                    // to the client that another page is available.  This item ID is stored at 0x32.
                    packet.List.itemnum_next = recipes[index]->Results[0].first;
                    break;
                }

                packet.List.itemnum[idx] = recipes[index]->Results[0].first;
                ++idx;
            }

            packet.List.Type = GP_SERV_COMMAND_RECIPE_TYPE::RecipeList;
            break;
        }
        default:
            break;
    }
}
