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

#include "lua/luautils.h"

#include "blue_trait.h"
#include "data/datasets/blue_traits/dataset.h"
#include "entities/battle_entity.h"
#include "trait.h"
#include "utils/dataset_loader.h"

#include <algorithm>
#include <utility>
#include <vector>

/************************************************************************
 *                                                                       *
 *                                                                       *
 *                                                                       *
 ************************************************************************/
CTrait::CTrait(const uint16 id)
: m_id(id)
{
}

/************************************************************************
 *                                                                       *
 *  Namespace for trait loading                                          *
 *                                                                       *
 ************************************************************************/
namespace traits
{

namespace
{

TraitList_t PTraitsList[MAX_JOBTYPE]; // Trait lists by job

} // namespace

/************************************************************************
 *                                                                       *
 *  LoadTraitList                                                        *
 *                                                                       *
 ************************************************************************/
void LoadTraitsList()
{
    auto rset = db::preparedStmt("SELECT traitid, job, level, rank, modifier, value, content_tag, meritid "
                                 "FROM traits "
                                 "WHERE traitid < ? "
                                 "ORDER BY job, traitid ASC, rank DESC",
                                 MAX_TRAIT_ID);
    FOR_DB_MULTIPLE_RESULTS(rset)
    {
        if (!luautils::IsContentEnabled(rset->getOrDefault<std::string>("content_tag", "")))
        {
            continue;
        }

        auto* PTrait = new CTrait(rset->get<uint16>("traitid"));

        PTrait->setJob(rset->get<int8>("job"));
        PTrait->setLevel(rset->get<uint8>("level"));
        PTrait->setRank(rset->get<uint8>("rank"));
        PTrait->setMod(rset->get<xi::Mod>("modifier"));
        PTrait->setValue(rset->get<int16>("value"));
        PTrait->setMeritId(rset->get<uint32>("meritid"));

        PTraitsList[PTrait->getJob()].emplace_back(PTrait);
    }

    const auto blueTraits = xi::data::loadDataset<xi::data::datasets::blue_traits::Dataset>();

    std::vector<std::pair<uint8, const xi::data::BlueTraitTierData*>> blueTiers;
    for (const auto& category : blueTraits)
    {
        for (const auto& tier : category.Tiers)
        {
            if (tier.Trait < MAX_TRAIT_ID)
            {
                blueTiers.emplace_back(category.Id, &tier);
            }
        }
    }

    std::ranges::stable_sort(blueTiers, {}, [](const auto& entry)
                             {
                                 return entry.second->Rank;
                             });

    for (const auto& [category, tier] : blueTiers)
    {
        for (const auto& [mod, value] : tier->Mods)
        {
            auto* PTrait = new CBlueTrait(category, static_cast<uint8>(tier->Trait), tier->JobPointsOnly);

            PTrait->setJob(static_cast<int8>(xi::Job::BLU));
            PTrait->setRank(tier->Rank);
            PTrait->setPoints(tier->Points);
            PTrait->setMod(mod);
            PTrait->setValue(value);

            PTraitsList[static_cast<uint8>(xi::Job::BLU)].emplace_back(PTrait);
        }
    }
}

void ClearTraitsList()
{
    // Manually cleanup traits list
    for (auto jobTraitList : PTraitsList)
    {
        for (auto trait : jobTraitList)
        {
            destroy(trait);
        }
        jobTraitList.clear();
    }
}

/************************************************************************
 *                                                                       *
 *  Get List of Traits by Main Job or Sub Job                            *
 *                                                                       *
 ************************************************************************/

auto GetTraits(xi::Job JobID) -> TraitList_t*
{
    if (static_cast<uint8>(JobID) >= MAX_JOBTYPE)
    {
        ShowWarning("JobID (%d) exceeds MAX_JOBTYPE.", static_cast<uint8>(JobID));
        return nullptr;
    }

    return &PTraitsList[static_cast<uint8>(JobID)];
}

}; // namespace traits
