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

#include "data/enums/mob_skill.h"
#include "data/enums/mob_skill_aoe.h"
#include "data/enums/mob_skill_flag.h"
#include "data/enums/skillchain_type.h"
#include "data/enums/target_type.h"
#include "map/data/datasets/mob_skills/dataset.h"
#include "map/data/yaml/merge.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace
{

using MobSkillsDataset = xi::data::datasets::mob_skills::Dataset;

auto slurp(const std::string& path) -> std::string
{
    std::ifstream input(path, std::ios::binary);
    return { std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };
}

auto findSkill(const xi::data::MobSkills& mobSkills, const xi::MobSkill id) -> const xi::data::MobSkillData&
{
    const auto it = std::ranges::find(mobSkills.Skills, id, &xi::data::MobSkillData::Id);
    REQUIRE(it != mobSkills.Skills.end());
    return *it;
}

auto findList(const xi::data::MobSkills& mobSkills, const uint16 id) -> const xi::data::MobSkillListData&
{
    const auto it = std::ranges::find(mobSkills.Lists, id, &xi::data::MobSkillListData::Id);
    REQUIRE(it != mobSkills.Lists.end());
    return *it;
}

} // namespace

TEST_CASE("mob_skills: skills read from data/mob_skills.yaml with defaults", "[data][mobskill]")
{
    const auto mobSkills = MobSkillsDataset::decode(slurp("data/mob_skills.yaml"));

    const auto& combo = findSkill(mobSkills, xi::MobSkill::Combo1);
    CHECK(combo.Name == "combo");
    CHECK(combo.Animation == 16);
    CHECK(combo.Aoe == xi::MobSkillAoe::Single);
    CHECK(combo.Distance == 7.0f);
    CHECK(combo.AnimationTime == std::chrono::milliseconds(2000));
    CHECK(combo.ReadyTime == std::chrono::milliseconds(0));
    CHECK(combo.Target == xi::TargetType::Enemy);
    CHECK(combo.Flags == xi::MobSkillFlag::None);
    CHECK(combo.Skillchain == std::vector{ xi::SkillchainType::Impaction });

    const auto& kickOut = findSkill(mobSkills, xi::MobSkill::KickOut);
    CHECK(kickOut.Name == "kick_out");
    CHECK(kickOut.Aoe == xi::MobSkillAoe::RearCone);
    CHECK(kickOut.Distance == 10.0f);
    CHECK(kickOut.ReadyTime == std::chrono::milliseconds(1500));

    CHECK(findSkill(mobSkills, xi::MobSkill::GrimHalo).Knockback == 2);
    CHECK(findSkill(mobSkills, xi::MobSkill::ArmorPiercerAutomaton).Param == 245);
}

TEST_CASE("mob_skills: lists resolve skill keys to ids in order", "[data][mobskill]")
{
    const auto mobSkills = MobSkillsDataset::decode(slurp("data/mob_skills.yaml"));

    const auto& adamantoise = findList(mobSkills, 2);
    CHECK(adamantoise.Name == "adamantoise");
    REQUIRE(adamantoise.Entries.size() == 6);
    CHECK(adamantoise.Entries.front() == std::to_underlying(xi::MobSkill::TortoiseSong1));

    CHECK(findList(mobSkills, 734).Entries == std::vector<uint16>{ 675, 676, 677, 678, 679 });
}

TEST_CASE("mob_skills: wotg era module patches skills and replaces jug pet lists", "[data][mobskill]")
{
    const auto modules   = std::array{ slurp("modules/era/data/wotg/mob_skills.yaml") };
    const auto mobSkills = MobSkillsDataset::decode(xi::data::mergeYaml(slurp("data/mob_skills.yaml"), modules));

    CHECK(findSkill(mobSkills, xi::MobSkill::ArmorPiercerAutomaton).Param == 999);
    CHECK(findSkill(mobSkills, xi::MobSkill::ArmorPiercerAutomaton).Distance == 15.0f);
    CHECK(findList(mobSkills, 734).Entries.front() == std::to_underlying(xi::MobSkill::HeadButtJugpet));
}

TEST_CASE("mob_skills: a list naming an unknown skill is rejected", "[data][mobskill]")
{
    const auto modules = std::array{ std::string{ "mob_skill_lists:\n  adamantoise:\n    skills: [not_a_mob_skill]\n" } };

    CHECK_THROWS(MobSkillsDataset::decode(xi::data::mergeYaml(slurp("data/mob_skills.yaml"), modules)));
}

TEST_CASE("mob_skills: a list cannot hold both skills and abilities", "[data][mobskill]")
{
    const auto modules = std::array{ std::string{ "mob_skill_lists:\n  jug_mandragora:\n    skills: [head_butt_jugpet]\n" } };

    CHECK_THROWS(MobSkillsDataset::decode(xi::data::mergeYaml(slurp("data/mob_skills.yaml"), modules)));
}
