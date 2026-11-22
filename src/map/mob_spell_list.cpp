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

#include <common/types/hash_map.h>

#include <algorithm>
#include <cstdlib>
#include <utility>

#include "data/datasets/mob_spell_lists/dataset.h"
#include "utils/dataset_loader.h"

#include "mob_spell_list.h"

CMobSpellList::CMobSpellList(Maybe<uint16> listId)
: m_listId(listId)
{
}

auto CMobSpellList::getId() const -> Maybe<uint16>
{
    return m_listId;
}

void CMobSpellList::AddSpell(const SpellID spellId, const uint16 minLvl, const uint16 maxLvl)
{
    MobSpell_t Mob_Spell = { spellId, minLvl, maxLvl };

    m_spellList.emplace_back(Mob_Spell);
}

auto CMobSpellList::GetSpellMinLevel(const SpellID spellId) const -> uint16
{
    if (const auto it = std::ranges::find(m_spellList, spellId, &MobSpell_t::spellId); it != m_spellList.end())
    {
        return it->min_level;
    }

    return 255;
}

// Implement namespace to work with spells
namespace mobSpellList
{

namespace
{

HashMap<uint16, std::unique_ptr<CMobSpellList>> PMobSpellList;

} // namespace

// Load list of spells
void LoadMobSpellList()
{
    PMobSpellList[0] = std::make_unique<CMobSpellList>(0); // Add empty spell list for mobSpellListId 0

    for (const auto& list : xi::data::loadDataset<xi::data::datasets::mob_spell_lists::Dataset>())
    {
        auto PSpellList = std::make_unique<CMobSpellList>(list.Id);

        for (const auto& entry : list.Spells)
        {
            const auto spellId = spell::lookupIdByName(entry.Name);
            if (!spellId)
            {
                ShowCriticalFmt("LoadMobSpellList: spell list '{}' names unknown spell '{}'", list.Name, entry.Name);
                std::exit(-1);
            }

            PSpellList->AddSpell(*spellId, entry.MinLevel, entry.MaxLevel);
        }

        std::ranges::sort(PSpellList->m_spellList, {}, [](const MobSpell_t& entry)
                          {
                              return std::pair{ entry.min_level, entry.spellId };
                          });

        PMobSpellList[list.Id] = std::move(PSpellList);
    }
}

// Get Spell By ID
auto GetMobSpellList(const uint16 mobSpellListId) -> CMobSpellList*
{
    if (PMobSpellList.contains(mobSpellListId))
    {
        return PMobSpellList[mobSpellListId].get();
    }

    ShowErrorFmt("Mob spell list ID {} does not exist.", mobSpellListId);
    return nullptr;
}

}; // namespace mobSpellList
