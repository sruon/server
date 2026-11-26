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

#include "data/enums/job.h"
#include "data/enums/job_point.h"
#include "data/enums/mod.h"
#include "map/data/datasets/job_points/dataset.h"
#include "map/data/yaml/merge.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace
{

using JobPointsDataset = xi::data::datasets::job_points::Dataset;

auto slurp(const std::string& path) -> std::string
{
    std::ifstream input(path, std::ios::binary);
    return { std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };
}

auto giftsAt(const xi::data::JobPointJobData& job, const uint16 spent) -> std::vector<xi::data::JobPointGiftData>
{
    std::vector<xi::data::JobPointGiftData> gifts;
    for (const auto& gift : job.Gifts)
    {
        if (gift.JobPointsSpent == spent)
        {
            gifts.emplace_back(gift);
        }
    }

    return gifts;
}

auto valueOf(const std::vector<xi::data::JobPointGiftData>& gifts, const xi::Mod mod) -> int16
{
    const auto it = std::ranges::find(gifts, mod, &xi::data::JobPointGiftData::Mod);
    REQUIRE(it != gifts.end());
    return it->Value;
}

} // namespace

TEST_CASE("job_points: decodes categories and gifts", "[data][jobpoints]")
{
    const auto jobPoints = JobPointsDataset::decode(slurp("data/job_points.yaml"));

    REQUIRE(jobPoints.size() == 22);
    for (const auto& [job, data] : jobPoints)
    {
        CHECK(data.Categories.size() == 10);
        CHECK(std::ranges::is_sorted(data.Gifts, {}, &xi::data::JobPointGiftData::JobPointsSpent));
    }

    const auto& warrior = jobPoints.at(xi::Job::WAR);
    CHECK(warrior.Categories.front() == xi::JobPoint::MightyStrikesEffect);
    CHECK(warrior.Categories.back() == xi::JobPoint::DoubleAttackEffect);
    CHECK(warrior.Gifts.front().JobPointsSpent == 5);

    const auto firstGifts = giftsAt(warrior, 5);
    CHECK(firstGifts.size() == 2);
    CHECK(valueOf(firstGifts, xi::Mod::DEF) == 10);
    CHECK(valueOf(firstGifts, xi::Mod::CAPACITY_BONUS) == 5);

    CHECK(jobPoints.at(xi::Job::RUN).Categories.front() == xi::JobPoint::ElementalSforzoEffect);
}

TEST_CASE("job_points: Superior gifts at 50, 100, 500, 1200 and 2100", "[data][jobpoints]")
{
    const auto jobPoints = JobPointsDataset::decode(slurp("data/job_points.yaml"));

    for (const auto& [job, data] : jobPoints)
    {
        std::vector<uint16> superiorAt;
        for (const auto& gift : data.Gifts)
        {
            if (gift.Mod == xi::Mod::SUPERIOR_LEVEL)
            {
                CHECK(gift.Value == 1);
                superiorAt.emplace_back(gift.JobPointsSpent);
            }
        }

        CAPTURE(xi::data::EnumTraits<xi::Job>::toName(job));
        CHECK(superiorAt == std::vector<uint16>{ 50, 100, 500, 1200, 2100 });
    }
}

TEST_CASE("job_points: COR magic accuracy and evasion gifts", "[data][jobpoints]")
{
    const auto  jobPoints = JobPointsDataset::decode(slurp("data/job_points.yaml"));
    const auto& corsair   = jobPoints.at(xi::Job::COR);

    CHECK(valueOf(giftsAt(corsair, 60), xi::Mod::MEVA) == 5);
    CHECK(valueOf(giftsAt(corsair, 80), xi::Mod::MACC) == 5);
}

TEST_CASE("job_points: Hasso and Seigan Effect is the zanshin cap", "[data][jobpoints]")
{
    const auto  jobPoints = JobPointsDataset::decode(slurp("data/job_points.yaml"));
    const auto& samurai   = jobPoints.at(xi::Job::SAM);

    for (const auto spent : std::array<uint16, 2>{ 100, 1200 })
    {
        const auto gifts = giftsAt(samurai, spent);
        CHECK(valueOf(gifts, xi::Mod::ZANSHIN_CAP) == 5);
        CHECK(std::ranges::find(gifts, xi::Mod::ZANSHIN, &xi::data::JobPointGiftData::Mod) == gifts.end());
    }
}

TEST_CASE("job_points: module changes, removes and adds gifts", "[data][jobpoints]")
{
    const auto modules   = std::array{ std::string{ "job_points:\n  war:\n    gifts:\n      100:\n        crithitrate: 6\n        superior_level: null\n      101:\n        str: 3\n" } };
    const auto jobPoints = JobPointsDataset::decode(xi::data::mergeYaml(slurp("data/job_points.yaml"), modules));

    const auto& warrior     = jobPoints.at(xi::Job::WAR);
    const auto  patchedTier = giftsAt(warrior, 100);
    CHECK(patchedTier.size() == 1);
    CHECK(valueOf(patchedTier, xi::Mod::CRITHITRATE) == 6);
    CHECK(valueOf(giftsAt(warrior, 101), xi::Mod::STR) == 3);
    CHECK(std::ranges::is_sorted(warrior.Gifts, {}, &xi::data::JobPointGiftData::JobPointsSpent));
}

TEST_CASE("job_points: category outside its job throws", "[data][jobpoints]")
{
    const auto modules = std::array{ std::string{ "job_points:\n  mnk:\n    categories:\n      mighty_strikes_effect: {id: 0x020}\n" } };

    CHECK_THROWS(JobPointsDataset::decode(xi::data::mergeYaml(slurp("data/job_points.yaml"), modules)));
}
