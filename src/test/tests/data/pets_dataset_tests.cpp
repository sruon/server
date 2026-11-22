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

#include "data/enums/pet_id.h"
#include "data/enums/pet_skill.h"
#include "data/enums/skill_flag.h"
#include "data/enums/target_type.h"
#include "map/data/datasets/pet_names/dataset.h"
#include "map/data/datasets/pet_skills/dataset.h"
#include "map/data/datasets/pets/dataset.h"
#include "map/data/yaml/merge.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <fstream>
#include <iterator>
#include <string>
#include <variant>
#include <vector>

namespace
{

using PetsDataset      = xi::data::datasets::pets::Dataset;
using PetNamesDataset  = xi::data::datasets::pet_names::Dataset;
using PetSkillsDataset = xi::data::datasets::pet_skills::Dataset;

auto slurp(const std::string& path) -> std::string
{
    std::ifstream input(path, std::ios::binary);
    return { std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };
}

auto find(const xi::data::Pets& pets, const xi::PetId id) -> const xi::data::PetData&
{
    const auto it = std::ranges::find(pets, id, &xi::data::PetData::Id);
    REQUIRE(it != pets.end());
    return *it;
}

auto find(const xi::data::PetSkills& skills, const xi::PetSkill id) -> const xi::data::PetSkillData&
{
    const auto it = std::ranges::find(skills, id, &xi::data::PetSkillData::Id);
    REQUIRE(it != skills.end());
    return *it;
}

} // namespace

TEST_CASE("pets: read from data/pets.yaml", "[data][pets]")
{
    const auto pets = PetsDataset::decode(slurp("data/pets.yaml"));

    REQUIRE(std::ranges::is_sorted(pets, {}, &xi::data::PetData::Id));

    const auto& carbuncle = find(pets, xi::PetId::Carbuncle);
    CHECK(carbuncle.Name == "Carbuncle");
    CHECK(carbuncle.Pool == 4588);
    CHECK(carbuncle.MinLevel == 1);
    CHECK(carbuncle.MaxLevel == 99);
    CHECK(carbuncle.Duration == std::chrono::seconds(0));
    CHECK(carbuncle.Element == xi::Element::Light);
    CHECK(carbuncle.DamageType == xi::DamageType::Slashing);

    const auto& sheep = find(pets, xi::PetId::SheepFamiliar);
    CHECK(sheep.MinLevel == 23);
    CHECK(sheep.MaxLevel == 35);
    CHECK(sheep.Duration == std::chrono::seconds(3600));
    CHECK(sheep.Element == xi::Element::None);
    CHECK(sheep.DamageType == xi::DamageType::Blunt);

    CHECK(find(pets, xi::PetId::CaitSith).Name == "Cait Sith");
    CHECK(find(pets, xi::PetId::Luopan).DamageType == xi::DamageType::None);
}

TEST_CASE("pet_names: read from data/pet_names.yaml", "[data][pets]")
{
    const auto names = PetNamesDataset::decode(slurp("data/pet_names.yaml"));

    CHECK(names.at(1) == "Azure");
    CHECK(names.at(288) == "Mk.IV");
    CHECK_FALSE(names.contains(0));
}

TEST_CASE("pet_skills: read from data/pet_skills.yaml", "[data][pets]")
{
    const auto skills = PetSkillsDataset::decode(slurp("data/pet_skills.yaml"));

    const auto& healingRuby = find(skills, xi::PetSkill::HealingRuby);
    CHECK(healingRuby.Name == "healing_ruby");
    CHECK(healingRuby.Range == 20.0f);
    CHECK(healingRuby.Target == (xi::TargetType::Self | xi::TargetType::PlayerParty));
    CHECK(healingRuby.Flags == xi::SkillFlag::BloodpactWard);
    CHECK(healingRuby.FinishCategory == 13);
    CHECK(healingRuby.AnimationTime == std::chrono::milliseconds(2000));
    CHECK(healingRuby.PrepareTime == std::chrono::milliseconds(1000));
    CHECK_FALSE(healingRuby.Area.has_value());

    const auto& shiningRuby = find(skills, xi::PetSkill::ShiningRuby);
    REQUIRE(shiningRuby.Area.has_value());
    CHECK(std::get<xi::data::AoeSphereData>(*shiningRuby.Area).Origin == xi::SphereOrigin::Caster);
    CHECK(std::get<xi::data::AoeSphereData>(*shiningRuby.Area).Radius == 14.0f);
    CHECK(shiningRuby.Flags == (xi::SkillFlag::NoTpCost | xi::SkillFlag::BloodpactWard));

    const auto& footKick = find(skills, xi::PetSkill::FootKick);
    CHECK(footKick.MobSkill == 3840);
    CHECK(footKick.FinishCategory == 11);
    CHECK(footKick.Target == xi::TargetType::Enemy);
    CHECK(footKick.Skillchain == std::vector{ xi::SkillchainType::Reverberation });

    const auto& dustCloud = find(skills, xi::PetSkill::DustCloud);
    REQUIRE(dustCloud.Area.has_value());
    CHECK(std::get<xi::data::AoeConeData>(*dustCloud.Area).Distance == 10.0f);

    CHECK(find(skills, xi::PetSkill::RegalGash).FinishCategory == 11);
    CHECK(find(skills, xi::PetSkill::LevelXHoly).Name == "level_X_holy");
    CHECK(find(skills, xi::PetSkill::HiFreqField).Name == "hi-freq_field");
    CHECK(find(skills, xi::PetSkill::Secretion).MasterRange == 8.0f);
}

TEST_CASE("pet_skills: era modules revert blood pacts, breaths and ready moves", "[data][pets]")
{
    const auto modules = std::array{ slurp("modules/era/data/soa/pet_skills.yaml"), slurp("modules/era/data/rov/pet_skills.yaml") };
    const auto skills  = PetSkillsDataset::decode(xi::data::mergeYaml(slurp("data/pet_skills.yaml"), modules));

    const auto& shiningRuby = find(skills, xi::PetSkill::ShiningRuby);
    CHECK(std::get<xi::data::AoeSphereData>(*shiningRuby.Area).Origin == xi::SphereOrigin::Caster);
    CHECK(std::get<xi::data::AoeSphereData>(*shiningRuby.Area).Radius == 10.0f);
    CHECK(shiningRuby.PrepareTime == std::chrono::milliseconds(3000));

    CHECK(find(skills, xi::PetSkill::FlamingCrush).Skillchain.empty());
    CHECK(find(skills, xi::PetSkill::FootKick).Skillchain.empty());
    CHECK(find(skills, xi::PetSkill::HealingBreath).PrepareTime == std::chrono::milliseconds(3000));
    CHECK(find(skills, xi::PetSkill::WildCarrot).AnimationTime == std::chrono::milliseconds(5000));
}

TEST_CASE("pet_skills: a module patch changes one field and keeps the rest", "[data][pets]")
{
    const auto modules = std::array{ std::string{ "pet_skills:\n  ifrit:\n    punch: {range: 5, skillchain: [fusion]}\n" } };
    const auto skills  = PetSkillsDataset::decode(xi::data::mergeYaml(slurp("data/pet_skills.yaml"), modules));

    const auto& punch = find(skills, xi::PetSkill::Punch);
    CHECK(punch.Range == 5.0f);
    CHECK(punch.Skillchain == std::vector{ xi::SkillchainType::Fusion });
    CHECK(punch.Flags == xi::SkillFlag::BloodpactRage);
}

TEST_CASE("pet_skills: invalid areas are rejected", "[data][pets]")
{
    const auto core = slurp("data/pet_skills.yaml");

    const auto rearCone = std::array{ std::string{ "pet_skills:\n  jug_pets:\n    dust_cloud:\n      aoe: {cone: {origin: rear}}\n" } };
    CHECK_THROWS(PetSkillsDataset::decode(xi::data::mergeYaml(core, rearCone)));

    const auto both = std::array{ std::string{ "pet_skills:\n  jug_pets:\n    dust_cloud:\n      aoe: {sphere: {origin: caster, radius: 4}}\n" } };
    CHECK_THROWS(PetSkillsDataset::decode(xi::data::mergeYaml(core, both)));

    const auto masterRangeWithArea = std::array{ std::string{ "pet_skills:\n  jug_pets:\n    dust_cloud: {master_range: 8}\n" } };
    CHECK_THROWS(PetSkillsDataset::decode(xi::data::mergeYaml(core, masterRangeWithArea)));
}
