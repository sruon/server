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

#include "synth_suggestion.h"

#include "common/database.h"
#include "common/logging.h"
#include "map_engine.h"

#include <map>

CSynthSuggestionListPacket::CSynthSuggestionListPacket(const uint16 skillID, const uint16 skillLevel, const uint8 skillRank, uint16 resultOffset)
{
    this->setType(0x31);
    this->setSize(0x34);

    ref<uint8>(0x0A) = skillLevel;

    const char* craftName = craftSkillDbNames[skillID - 1].c_str();
    uint8       minSkill  = skillRank * 10;
    uint8       maxSkill  = (skillRank + 1) * 10;

    if (skillLevel < maxSkill)
    {
        maxSkill = skillLevel;
    }

    const auto query        = "SELECT Result "
                              "FROM synth_recipes "
                              "INNER JOIN item_basic ON Result = item_basic.itemid "
                              "WHERE ? >= GREATEST(`Wood`, `Smith`, `Gold`, `Cloth`, `Leather`, `Bone`, `Alchemy`, `Cook`) AND "
                              "? BETWEEN ? AND ? "
                              "AND Desynth = 0 "
                              "ORDER BY ?, item_basic.name "
                              "LIMIT ?, 17";
    const auto rset         = db::preparedStmt(query, craftName, craftName, minSkill, maxSkill, craftName, resultOffset);
    uint8      itemIdOffset = 0x10;
    FOR_DB_MULTIPLE_RESULTS(rset)
    {
        ref<uint16>(itemIdOffset) = rset->get<uint16>("Result");

        itemIdOffset += 2;
        if (itemIdOffset == 0x30)
        {
            // The 17th result of a query is not displayed in the menu, but instead is used to signal
            // to the client that another page is available.  This item ID is stored at 0x32.
            itemIdOffset += 2;
        }
    }

    ref<uint16>(0x30) = 0x02;
}

CSynthSuggestionRecipePacket::CSynthSuggestionRecipePacket(const uint16 skillID, const uint16 skillLevel, const uint8 skillRank, uint16 selectedRecipeOffset)
{
    this->setType(0x31);
    this->setSize(0x34);

    const char* craftName = craftSkillDbNames[skillID - 1].c_str();
    uint8       minSkill  = skillRank * 10;
    uint8       maxSkill  = (skillRank + 1) * 10;

    if (skillLevel < maxSkill)
    {
        maxSkill = skillLevel;
    }

    const auto query = "SELECT KeyItem, Wood, Smith, Gold, Cloth, Leather, Bone, Alchemy, Cook, Crystal, Result,  "
                       "Ingredient1, Ingredient2, Ingredient3, Ingredient4, Ingredient5, Ingredient6, Ingredient7, Ingredient8 "
                       "FROM synth_recipes INNER JOIN item_basic ON Result = item_basic.itemid "
                       "WHERE ? >= GREATEST(`Wood`, `Smith`, `Gold`, `Cloth`, `Leather`, `Bone`, `Alchemy`, `Cook`) AND "
                       "? BETWEEN ? AND ? AND Desynth = 0 ORDER BY ?, item_basic.name LIMIT ?, 1";
    const auto rset  = db::preparedStmt(query, craftName, craftName, minSkill, maxSkill, craftName, selectedRecipeOffset);
    FOR_DB_SINGLE_RESULT(rset)
    {
        std::map<uint16, uint16> ingredients;
        uint16                   subcraftIDs[3] = { 0u, 0u, 0u };
        size_t                   subidx         = 0;

        // So, each craft can have up to 3 subcrafts. This loop is
        //     to pack the subcraft requirements to be sent
        static const char* craftColumns[] = {
            nullptr, "Wood", "Smith", "Gold", "Cloth", "Leather", "Bone", "Alchemy", "Cook"
        };
        
        for (auto i = 1; i < 9; ++i)
        {
            uint16 this_skill = 0u;
            if (i != skillID && subidx < 3)
            {
                this_skill = rset->get<uint16>(craftColumns[i]);
            }

            if (this_skill > 0u)
            {
                subcraftIDs[subidx] = i;
                subidx++;
            }
        }

        ref<uint16>(0x04) = rset->get<uint16>("Result");
        ref<uint16>(0x06) = subcraftIDs[0];
        ref<uint16>(0x08) = subcraftIDs[1];
        ref<uint16>(0x0A) = subcraftIDs[2];
        ref<uint16>(0x0C) = rset->get<uint16>("Crystal");
        ref<uint16>(0x0E) = rset->get<uint16>("KeyItem");

        // So this loop is a little weird. What we store in the db
        //     is a list of 8 individual ingredients which may or
        //     may not contain duplicates. What we need for the
        //     packet is a set of ingredient and quantity. In order
        //     to achieve that, we're pushing the first instance of
        //     an ingredient into a std::map with a qty 1 and then
        //     any duplicate instances will increase the quantity
        //     without creating new duplicate entries
        for (auto i = 0; i < 8; ++i)
        {
            uint16 this_ingredient = 0;
            std::string ingredientColumn = fmt::format("Ingredient{}", i + 1);

            this_ingredient = rset->get<uint16>(ingredientColumn);
            if (this_ingredient != 0)
            {
                if (ingredients[this_ingredient])
                {
                    ingredients[this_ingredient] = ingredients[this_ingredient] + 1;
                }
                else
                {
                    ingredients[this_ingredient] = 1;
                }
            }
        }

        // Finally, store the contents of the map of ingredients
        //     into the proper offsets in the packet before sending
        uint8 pointer_ref = 0x10u;
        for (const auto& ingredient : ingredients)
        {
            ref<uint16>(pointer_ref)        = ingredient.first;
            ref<uint16>(pointer_ref + 0x10) = ingredient.second;
            pointer_ref += 0x02u;
            if (pointer_ref > 0x1E)
            {
                break;
            }
        }
        ref<uint16>(0x30) = 0x01;
    }
}
