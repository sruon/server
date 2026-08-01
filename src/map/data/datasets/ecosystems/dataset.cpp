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

#include "data/datasets/ecosystems/dataset.h"

#include "data/datasets/ecosystems/yaml.h"
#include "data/yaml/read.h"

#include <fmt/format.h>

#include <optional>
#include <stdexcept>
#include <utility>

namespace xi::data::datasets::ecosystems
{

namespace
{

static_assert(glz::reflect<wire::StatRanks>::size == 11,
              "update the stat-rank resolver when the wire shape changes");
static_assert(glz::reflect<StatRanksOverrides>::size == 11,
              "update the stat-rank resolver when the domain shape changes");

// Convert wire stat ranks into domain overrides.
auto resolveRanks(const wire::StatRanks& input) -> StatRanksOverrides
{
    const auto resolve = []<class Enum>(const std::optional<yaml::EnumToken<Enum>>& token) -> std::optional<Enum>
    {
        return token ? std::optional{ yaml::resolveEnum(*token) } : std::nullopt;
    };

    return {
        .Str = resolve(input.str),
        .Dex = resolve(input.dex),
        .Vit = resolve(input.vit),
        .Agi = resolve(input.agi),
        .Int = resolve(input.intelligence),
        .Mnd = resolve(input.mnd),
        .Chr = resolve(input.chr),
        .Def = resolve(input.def),
        .Eva = resolve(input.eva),
        .Att = resolve(input.att),
        .Acc = resolve(input.acc),
    };
}

static_assert(glz::reflect<wire::MobAttributes>::size == 5,
              "update the mob-attribute resolver when the wire shape changes");
static_assert(glz::reflect<MobAttributesOverrides>::size == 5,
              "update the mob-attribute resolver when the domain shape changes");

// Convert wire attributes into domain overrides.
auto resolveAttributes(const wire::MobAttributes& input) -> MobAttributesOverrides
{
    MobAttributesOverrides result{};
    if (input.element)
    {
        result.Element = yaml::resolveEnum(*input.element);
    }
    if (input.stats)
    {
        result.Stats = resolveRanks(*input.stats);
    }
    if (input.detects)
    {
        result.Detects = yaml::resolveFlags(input.detects);
    }
    result.Speed     = input.speed;
    result.Charmable = input.charmable;
    return result;
}

// Treat a missing attribute block as no overrides.
auto resolveAttributes(const std::optional<wire::MobAttributes>& input) -> MobAttributesOverrides
{
    return input ? resolveAttributes(*input) : MobAttributesOverrides{};
}

static_assert(glz::reflect<wire::Species>::size == 2,
              "update the species resolver when the wire shape changes");

// Index species by numeric ID.
auto resolveSpecies(const std::optional<std::map<std::string, wire::Species>>& input)
    -> HashMap<xi::Species, SpeciesData>
{
    if (!input)
    {
        return {};
    }

    HashMap<xi::Species, SpeciesData> result;
    for (const auto& [key, value] : *input)
    {
        yaml::verifyNamedMapEntry<xi::Species>(key, value.id);
        if (value.id == 0)
        {
            throw std::runtime_error(fmt::format("species '{}' declares reserved id 0", key));
        }
        const auto id = static_cast<xi::Species>(value.id);
        if (!result.try_emplace(id, SpeciesData{ id, resolveAttributes(value.attributes) }).second)
        {
            throw std::runtime_error(fmt::format("duplicate species id {}", value.id));
        }
    }
    return result;
}

static_assert(glz::reflect<wire::Family>::size == 3,
              "update the family resolver when the wire shape changes");

// Index families by numeric ID.
auto resolveFamilies(const std::optional<std::map<std::string, wire::Family>>& input)
    -> HashMap<xi::Family, FamilyData>
{
    if (!input)
    {
        return {};
    }

    HashMap<xi::Family, FamilyData> result;
    for (const auto& [key, value] : *input)
    {
        yaml::verifyNamedMapEntry<xi::Family>(key, value.id);
        if (value.id == 0)
        {
            throw std::runtime_error(fmt::format("family '{}' declares reserved id 0", key));
        }
        const auto id = static_cast<xi::Family>(value.id);
        FamilyData family{ id, resolveAttributes(value.attributes), resolveSpecies(value.species) };
        if (!result.try_emplace(id, std::move(family)).second)
        {
            throw std::runtime_error(fmt::format("duplicate family id {}", value.id));
        }
    }
    return result;
}

} // namespace

static_assert(glz::reflect<wire::Ecosystem>::size == 3,
              "update the ecosystem resolver when the wire shape changes");

// Decode and index ecosystems by numeric ID.
auto Dataset::decode(const std::string_view input) -> Records
{
    const auto file = yaml::read<YamlFile>(input);
    Records    result;

    for (const auto& [key, value] : file.ecosystems)
    {
        yaml::verifyNamedMapEntry<xi::Ecosystem>(key, value.id);
        const auto    id = static_cast<xi::Ecosystem>(value.id);
        EcosystemData ecosystem{ id, resolveAttributes(value.attributes), resolveFamilies(value.families) };
        if (!result.try_emplace(id, std::move(ecosystem)).second)
        {
            throw std::runtime_error(fmt::format("duplicate ecosystem id {}", value.id));
        }
    }
    return result;
}

} // namespace xi::data::datasets::ecosystems
