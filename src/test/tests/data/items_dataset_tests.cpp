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

// The YAML shapes that decode into the packed values item templates used to read from SQL.

#include "data/enums/item.h"
#include "data/enums/job.h"
#include "map/data/datasets/items/dataset.h"
#include "map/data/yaml/merge.h"
#include "map/utils/dataset_loader.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

namespace
{

using ItemsDataset = xi::data::datasets::items::Dataset;

auto item(const xi::Item id) -> const xi::data::ItemData&
{
    static const auto loaded = xi::data::loadDataset<ItemsDataset>();
    return loaded.at(std::to_underlying(id));
}

} // namespace

TEST_CASE("items: usable blocks carry targets and timings", "[data][item]")
{
    const auto& potion = item(xi::Item::Potion);

    CHECK(potion.Type == xi::ItemType::Usable);
    REQUIRE(potion.Usable.has_value());
    CHECK(potion.Usable->ValidTargets == xi::TargetType::Self);
    CHECK(potion.Usable->Activation == std::chrono::milliseconds(1000));
    CHECK(potion.Usable->Animation == 30);
    CHECK(potion.Stack == 1);
}

TEST_CASE("items: jobs all is WAR through RUN", "[data][item]")
{
    const auto& shirt = item(xi::Item::ChocoboShirt);

    REQUIRE(shirt.Equipment.has_value());
    CHECK(shirt.Equipment->Jobs == (1U << 22) - 1);
    CHECK((shirt.Equipment->Jobs & (1U << (std::to_underlying(xi::Job::MON) - 1))) == 0);
    CHECK(shirt.Equipment->Slots == xi::EquipSlotFlag::Body);

    REQUIRE(shirt.Equipment->Latents.size() == 3);
    CHECK(shirt.Equipment->Latents[0].Condition == xi::Latent::JobLevelBelow);
    CHECK(shirt.Equipment->Latents[0].Param == 31);
    CHECK(shirt.Equipment->Latents[0].Value == 50);
}

TEST_CASE("items: job lists become the equipment job mask", "[data][item]")
{
    const auto& cesti = item(xi::Item::Cesti);
    const auto  bit   = [](const xi::Job job)
    {
        return 1U << (std::to_underlying(job) - 1);
    };

    REQUIRE(cesti.Equipment.has_value());
    CHECK((cesti.Equipment->Jobs & bit(xi::Job::WAR)) != 0);
    CHECK((cesti.Equipment->Jobs & bit(xi::Job::DNC)) != 0);
    CHECK((cesti.Equipment->Jobs & bit(xi::Job::WHM)) == 0);
    CHECK(cesti.Equipment->Slots == xi::EquipSlotFlag::Main);
    CHECK(cesti.Equipment->Mods.at(xi::Mod::ACC) == 3);

    REQUIRE(cesti.Weapon.has_value());
    CHECK(cesti.Weapon->Skill == xi::SkillType::HandToHand);
    CHECK(cesti.Weapon->DamageType == xi::DamageType::HandToHand);
    CHECK(cesti.Weapon->Delay == 528);
    CHECK(cesti.Weapon->Damage == 1);
    CHECK(cesti.Weapon->Hits == 1);
}

TEST_CASE("items: pet mods group by pet", "[data][item]")
{
    const auto& earring = item(xi::Item::CharivariEarring);

    REQUIRE(earring.Equipment.has_value());
    const auto& automaton = earring.Equipment->PetMods.at(xi::PetModType::Automaton);
    CHECK(automaton.at(xi::Mod::ACC) == 3);
    CHECK(automaton.at(xi::Mod::MACC) == 3);
}

TEST_CASE("items: puppet elements pack one nibble per element", "[data][item]")
{
    const auto& head = item(xi::Item::HarlequinHead);

    REQUIRE(head.Puppet.has_value());
    CHECK(head.Puppet->Slot == 1);
    CHECK(head.Puppet->ElementSlots == 0x222222);
}

TEST_CASE("items: furnishing blocks keep placement and footprint", "[data][item]")
{
    const auto& bedding = item(xi::Item::PileOfChocoboBedding);

    CHECK(bedding.SortName == "chocobo_bedding");
    REQUIRE(bedding.Furnishing.has_value());
    CHECK(bedding.Furnishing->Element == xi::Element::Dark);
    CHECK(bedding.Furnishing->Placement == xi::FurnishingPlacement::Floor);
    CHECK(bedding.Furnishing->SizeX == 5);
    CHECK(bedding.Furnishing->SizeY == 5);
    CHECK(bedding.Furnishing->Height == 160);
}

TEST_CASE("items: the era item patch overrides its fields and keeps the rest", "[data][item]")
{
    const std::vector<std::string> modules{ "modules/era/data/abyssea/items/items.yaml" };

    const auto  patched = ItemsDataset::decode(xi::data::loadMergedYaml("data/items.yaml", modules));
    const auto& cap     = patched.at(std::to_underlying(xi::Item::MoogleCap));

    REQUIRE(cap.Usable.has_value());
    CHECK(cap.Usable->ReuseDelay == std::chrono::seconds(259200));
    CHECK(cap.Usable->ValidTargets == item(xi::Item::MoogleCap).Usable->ValidTargets);
    CHECK(cap.Usable->MaxCharges == item(xi::Item::MoogleCap).Usable->MaxCharges);
    CHECK(cap.Equipment.has_value());
    CHECK(item(xi::Item::MoogleCap).Usable->ReuseDelay == std::chrono::seconds(72000));

    const auto& biscuit = patched.at(std::to_underlying(xi::Item::PetFoodAlphaBiscuit));
    REQUIRE(biscuit.Equipment.has_value());
    CHECK(biscuit.Equipment->Level == 0);
    CHECK(biscuit.Equipment->Jobs == item(xi::Item::PetFoodAlphaBiscuit).Equipment->Jobs);

    const auto& spring = patched.at(std::to_underlying(xi::Item::TensionSpring8450));
    REQUIRE(spring.Puppet.has_value());
    CHECK(spring.Puppet->ElementSlots == 0x2);
    CHECK(spring.Puppet->Slot == item(xi::Item::TensionSpring8450).Puppet->Slot);

    CHECK(patched.at(std::to_underlying(xi::Item::Potion)).Usable->Animation == item(xi::Item::Potion).Usable->Animation);
}

TEST_CASE("items: the era item patch only applies when its folder is listed", "[data][item]")
{
    const auto original = std::filesystem::current_path();
    const auto root     = std::filesystem::temp_directory_path() / "xi_test_item_modules";
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root / "modules/era/data/abyssea/items");
    std::ofstream(root / "modules/era/data/abyssea/items/items.yaml") << "items: {}\n";

    const auto pathsFor = [&](const std::string& entry)
    {
        std::ofstream(root / "modules/init.txt") << entry << "\n";
        std::filesystem::current_path(root);
        auto paths = xi::data::getDataModulePaths("items", ".yaml");
        std::filesystem::current_path(original);
        return paths;
    };

    CHECK(pathsFor("era").empty());
    CHECK(pathsFor("era/sql/abyssea").empty());
    CHECK(pathsFor("era/data/abyssea/items") == std::vector<std::string>{ "./modules/era/data/abyssea/items/items.yaml" });

    std::filesystem::remove_all(root);
}
