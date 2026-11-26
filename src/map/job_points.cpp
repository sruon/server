/*
===========================================================================
  Copyright (c) 2021 Ixion Dev Teams
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

#include "job_points.h"
#include "data/datasets/job_points/dataset.h"
#include "entities/battle_entity.h"
#include "entities/char_entity.h"

#include "packets/s2c/0x0aa_magic_data.h"
#include "utils/charutils.h"
#include "utils/dataset_loader.h"

#include <fmt/ranges.h>

#include <set>
#include <string>
#include <unordered_map>

namespace
{

xi::data::JobPoints                     jobPointData{};
std::unordered_map<std::string, uint16> giftSpellIds;

} // namespace

CJobPoints::CJobPoints(CCharEntity* PChar)
: m_PChar(PChar)
{
    this->LoadJobPoints();
}

void CJobPoints::LoadJobPoints()
{
    const auto rset = db::preparedStmt("SELECT charid, jobid, capacity_points, job_points, "
                                       "job_points_spent, jptype0, jptype1, jptype2, "
                                       "jptype3, jptype4, jptype5, jptype6, "
                                       "jptype7, jptype8, jptype9 "
                                       "FROM char_job_points "
                                       "WHERE charid = ? "
                                       "ORDER BY jobid ASC",
                                       m_PChar->id);
    FOR_DB_MULTIPLE_RESULTS(rset)
    {
        const auto   jobId       = rset->get<uint32>("jobid");
        const uint16 jobCategory = JobPointsCategoryByJobId(jobId);
        JobPoints_t  currentJob  = {};

        currentJob.jobId          = jobId;
        currentJob.jobCategory    = jobCategory;
        currentJob.capacityPoints = rset->get<uint16>("capacity_points");
        currentJob.currentJp      = rset->get<uint16>("job_points");
        currentJob.totalJpSpent   = rset->get<uint16>("job_points_spent");

        for (uint8 j = 0; j < JOBPOINTS_JPTYPE_PER_CATEGORY; j++)
        {
            JobPointType_t currentType = {};
            currentType.id             = currentJob.jobCategory + j;
            currentType.value          = rset->get<uint8>(JOBPOINTS_SQL_COLUMN_OFFSET + j);
            std::memcpy(&currentJob.job_point_types[j], &currentType, sizeof(JobPointType_t));
        }

        std::memcpy(&m_jobPoints[jobId], &currentJob, sizeof(JobPoints_t));
    }
}

auto CJobPoints::IsJobPointExist(const xi::JobPoint jpType) -> bool
{
    if ((static_cast<uint16>(jpType) < JOBPOINTS_CATEGORY_START) ||
        (JobPointsCategoryIndexByJpType(jpType) - 1 >= JOBPOINTS_CATEGORY_COUNT) ||
        (JobPointTypeIndex(jpType) >= JOBPOINTS_JPTYPE_PER_CATEGORY))
    {
        return false;
    }

    return true;
}

auto CJobPoints::GetJobPointsByType(const xi::JobPoint jpType) -> JobPoints_t*
{
    if (IsJobPointExist(jpType))
    {
        return &m_jobPoints[JobPointsCategoryIndexByJpType(jpType)];
    }

    return nullptr;
}

auto CJobPoints::GetJobPointType(const xi::JobPoint jpType) -> JobPointType_t*
{
    if (IsJobPointExist(jpType))
    {
        return &m_jobPoints[JobPointsCategoryIndexByJpType(jpType)].job_point_types[JobPointTypeIndex(jpType)];
    }

    return nullptr;
}

void CJobPoints::RaiseJobPoint(const xi::JobPoint jpType)
{
    JobPoints_t*    job      = GetJobPointsByType(jpType);
    JobPointType_t* jobPoint = GetJobPointType(jpType);

    if (!job || !jobPoint)
    {
        return;
    }

    uint8 cost = JobPointCost(jobPoint->value);

    if (cost != 0 && job->currentJp >= cost)
    {
        job->currentJp -= cost;
        job->totalJpSpent += cost;
        jobPoint->value++;

        const auto& types = job->job_point_types;
        db::preparedStmt("UPDATE char_job_points SET "
                         "jptype0 = ?, jptype1 = ?, jptype2 = ?, jptype3 = ?, jptype4 = ?, "
                         "jptype5 = ?, jptype6 = ?, jptype7 = ?, jptype8 = ?, jptype9 = ?, "
                         "job_points = ?, job_points_spent = ? "
                         "WHERE charid = ? AND jobid = ?",
                         types[0].value,
                         types[1].value,
                         types[2].value,
                         types[3].value,
                         types[4].value,
                         types[5].value,
                         types[6].value,
                         types[7].value,
                         types[8].value,
                         types[9].value,
                         job->currentJp,
                         job->totalJpSpent,
                         m_PChar->id,
                         job->jobId);

        jobpointutils::RefreshGiftMods(m_PChar);
    }
}

auto CJobPoints::GetJobPoints() -> uint16
{
    return m_jobPoints[static_cast<uint8>(m_PChar->GetMJob())].currentJp;
}

auto CJobPoints::GetJobPointsByJob(uint8 jobID) const -> uint16
{
    const auto rset = db::preparedStmt("SELECT job_points FROM char_job_points WHERE charid=? AND jobid=? LIMIT 1", m_PChar->id, jobID);
    FOR_DB_SINGLE_RESULT(rset)
    {
        return rset->get<uint16>("job_points");
    }

    return 0;
}

void CJobPoints::SetJobPoints(int16 amount)
{
    const auto currentJob = static_cast<uint8>(m_PChar->GetMJob());
    amount                = std::clamp<int16>(amount, 0, 500);

    db::preparedStmt("INSERT INTO char_job_points "
                     "SET charid=?, jobid=?, job_points=? "
                     "ON DUPLICATE KEY UPDATE job_points=?",
                     m_PChar->id,
                     currentJob,
                     amount,
                     amount);

    LoadJobPoints();
}

void CJobPoints::AddJobPoints(uint8 jobID, uint16 amount)
{
    if (jobID == 0 || jobID > 22)
    {
        ShowDebug("Attempt to adjust job points for an invalid job for (%s).", m_PChar->getName());
        return;
    }

    amount = std::clamp<int16>(amount, 0, 500);
    db::preparedStmt("INSERT INTO char_job_points "
                     "SET charid=?, jobid=?, job_points=? "
                     "ON DUPLICATE KEY UPDATE job_points=job_points +?",
                     m_PChar->id,
                     jobID,
                     amount,
                     amount);

    LoadJobPoints();
}

void CJobPoints::DelJobPoints(const uint8 jobID, int16 amount)
{
    const int16 currentAmount = GetJobPointsByJob(jobID);
    amount                    = std::clamp<int16>(amount, -500, 500);
    if (currentAmount < amount)
    {
        ShowDebugFmt("Attempt to reduce job points below 0 for ({}).", m_PChar->getName());
        return;
    }

    db::preparedStmt("UPDATE char_job_points SET job_points=? WHERE charid=? AND jobid=?",
                     currentAmount - amount,
                     m_PChar->id,
                     jobID);

    LoadJobPoints();
}

auto CJobPoints::GetJobPointsSpent() const -> uint16
{
    return m_jobPoints[static_cast<uint8>(m_PChar->GetMJob())].totalJpSpent;
}

auto CJobPoints::GetAllJobPoints() -> JobPoints_t*
{
    return m_jobPoints;
}

auto CJobPoints::AddCapacityPoints(uint16 amount) -> bool
{
    uint32 adjustedCapacity = m_jobPoints[static_cast<uint8>(m_PChar->GetMJob())].capacityPoints + amount * settings::get<float>("map.CAPACITY_RATE");
    uint16 currentJobPoints = this->GetJobPoints();

    if (adjustedCapacity >= 30000)
    {
        // check if player has reached cap
        if (currentJobPoints == 500)
        {
            this->SetCapacityPoints(30000 - 1);
            return false;
        }

        uint16 jobPoints = std::min((int)(currentJobPoints + adjustedCapacity / 30000), 500);

        this->SetCapacityPoints(adjustedCapacity % 30000);

        if (currentJobPoints != jobPoints)
        {
            this->SetJobPoints(jobPoints);
            return true;
        }
    }
    else
    {
        this->SetCapacityPoints(adjustedCapacity);
    }

    return false;
}

auto CJobPoints::GetCapacityPoints() -> uint32
{
    return m_jobPoints[static_cast<uint8>(m_PChar->GetMJob())].capacityPoints;
}

void CJobPoints::SetCapacityPoints(uint16 amount)
{
    auto currentJob                        = static_cast<uint8>(m_PChar->GetMJob());
    amount                                 = std::clamp<int16>(amount, 0, 30000);
    m_jobPoints[currentJob].capacityPoints = amount;

    db::preparedStmt("INSERT INTO char_job_points "
                     "SET charid=?, jobid=?, capacity_points=? "
                     "ON DUPLICATE KEY UPDATE capacity_points=?",
                     m_PChar->id,
                     currentJob,
                     amount,
                     amount);
}

auto CJobPoints::GetJobPointValue(const xi::JobPoint jpType) -> uint8
{
    if (IsJobPointExist(jpType) && m_PChar->GetMLevel() >= 99 && static_cast<uint8>(m_PChar->GetMJob()) == JobPointsCategoryIndexByJpType(jpType))
    {
        return GetJobPointType(jpType)->value;
    }

    return 0;
}

namespace jobpointutils
{

void LoadGifts()
{
    jobPointData = xi::data::loadDataset<xi::data::datasets::job_points::Dataset>();

    std::unordered_map<std::string, uint16> spellIds;
    const auto                              rset = db::preparedStmt("SELECT spellid, name FROM spell_list");
    FOR_DB_MULTIPLE_RESULTS(rset)
    {
        spellIds.emplace(rset->get<std::string>("name"), rset->get<uint16>("spellid"));
    }

    giftSpellIds.clear();
    std::set<std::string> missing;
    for (const auto& [job, data] : jobPointData)
    {
        for (const auto& gift : data.GiftSpells)
        {
            if (const auto spell = spellIds.find(gift.Spell); spell != spellIds.end())
            {
                giftSpellIds.emplace(gift.Spell, spell->second);
            }
            else
            {
                missing.emplace(gift.Spell);
            }
        }
    }

    if (!missing.empty())
    {
        ShowWarningFmt("Job point gift spells missing from spell_list: {}", fmt::join(missing, ", "));
    }
}

void RefreshGiftMods(CCharEntity* PChar)
{
    const uint16 totalJpSpent = PChar->PJobPoints->GetJobPointsSpent();

    auto* currentGifts = &PChar->PJobPoints->current_gifts;
    if (!currentGifts->empty())
    {
        PChar->delModifiers(currentGifts);
        currentGifts->clear();
    }

    const auto job = jobPointData.find(PChar->GetMJob());
    if (job == jobPointData.end())
    {
        return;
    }

    for (const auto& gift : job->second.Gifts)
    {
        if (gift.JobPointsSpent > totalJpSpent || PChar->GetMLevel() < 99)
        {
            break;
        }

        currentGifts->emplace_back(gift.Mod, gift.Value);
    }

    PChar->addModifiers(currentGifts);

    bool learnedSpell = false;
    for (const auto& gift : job->second.GiftSpells)
    {
        if (gift.JobPointsSpent > totalJpSpent)
        {
            break;
        }

        const auto spell = giftSpellIds.find(gift.Spell);
        if (spell != giftSpellIds.end() && charutils::addSpell(PChar, spell->second))
        {
            charutils::SaveSpell(PChar, spell->second);
            learnedSpell = true;
        }
    }

    if (learnedSpell)
    {
        PChar->pushPacket<GP_SERV_COMMAND_MAGIC_DATA>(PChar);
    }
}

} // namespace jobpointutils
