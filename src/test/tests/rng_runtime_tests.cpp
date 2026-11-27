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

//
// Runtime companions to rng_spread_tests.cpp. The consteval tests there lock the
// raw engine's spread and boundaries at compile time; these exercise the parts
// that can't run in a constant expression: the thread-local engine plumbing
// (xirand::rng() / seeding), the libm-backed normal distribution, and the
// weighted/shuffle helpers. Contracts asserted here mirror the Lua-facing tests
// in scripts/tests/framework/prng.lua, one layer down.
//

#include <common/xirand.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/interfaces/catch_interfaces_config.hpp>
#include <catch2/internal/catch_context.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <future>
#include <iostream>
#include <limits>
#include <map>
#include <string_view>
#include <thread>
#include <vector>

namespace
{

constexpr int kSamples = 1'000'000;

constexpr char kStreams[]      = "seeded engine reproduces the same stream";
constexpr char kDistribution[] = "GetNormalNumber matches the requested distribution";

auto drawSeededStreams()
{
    std::vector<uint32_t> first;
    first.reserve(16);
    xirand::rng().seed(42);
    for (int i = 0; i < 16; ++i)
    {
        first.emplace_back(xirand::GetRandomNumber<uint32_t>(1'000'000));
    }

    std::vector<uint32_t> second;
    second.reserve(16);
    xirand::rng().seed(42);
    for (int i = 0; i < 16; ++i)
    {
        second.emplace_back(xirand::GetRandomNumber<uint32_t>(1'000'000));
    }

    return std::pair{ std::move(first), std::move(second) };
}

auto sampleNormalDistribution()
{
    xirand::rng().seed(1);

    double sum   = 0.0;
    double sumSq = 0.0;
    for (int i = 0; i < kSamples; ++i)
    {
        const auto value = xirand::GetNormalNumber(3.5, 1.5);
        sum += value;
        sumSq += value * value;
    }

    const double mean   = sum / kSamples;
    const double stddev = std::sqrt(sumSq / kSamples - mean * mean);

    return std::pair{ mean, stddev };
}

using Clock = std::chrono::steady_clock;

template <typename Result>
struct RngCaseResult
{
    Result               value;
    SelectedRandomEngine engine;
    const char*          name;
    std::thread::id      thread;
    Clock::time_point    start;
    Clock::time_point    end;
};

template <typename Function>
auto launchRngCase(const char* name, Function function)
{
    const auto work = [name, function]
    {
        const auto start = Clock::now();
        auto       value = function();
        const auto end   = Clock::now();
        return RngCaseResult{ std::move(value), xirand::rng(), name, std::this_thread::get_id(), start, end };
    };

    try
    {
        return std::async(std::launch::async, work);
    }
    catch (...)
    {
        std::promise<decltype(work())> failure;
        failure.set_exception(std::current_exception());
        return failure.get_future();
    }
}

struct ParallelRngCases
{
    std::future<RngCaseResult<decltype(sampleNormalDistribution())>> distribution = launchRngCase(kDistribution, sampleNormalDistribution);
    std::future<RngCaseResult<decltype(drawSeededStreams())>>        streams      = launchRngCase(kStreams, drawSeededStreams);

    ParallelRngCases()
    {
        distribution.wait();
        streams.wait();
    }
};

auto parallelRngCases() -> ParallelRngCases&
{
    static ParallelRngCases cases;
    return cases;
}

template <typename Function, typename Future>
auto runRngCase(Function function, Future ParallelRngCases::* member)
{
    const auto* mode   = std::getenv("XI_TEST_PARALLEL_RNG");
    const auto* config = Catch::getCurrentContext().getConfig();
    if (!mode || std::string_view(mode) != "1" || config->hasTestFilters() || config->shardCount() != 1 || config->abortAfter() > 0)
    {
        return function();
    }

    auto result   = (parallelRngCases().*member).get();
    xirand::rng() = std::move(result.engine);
    std::cerr << "RNG case \"" << result.name << "\" thread=" << result.thread
              << " start_ns=" << std::chrono::duration_cast<std::chrono::nanoseconds>(result.start.time_since_epoch()).count()
              << " end_ns=" << std::chrono::duration_cast<std::chrono::nanoseconds>(result.end.time_since_epoch()).count() << '\n';
    return std::move(result.value);
}

TEST_CASE(kStreams, "[rng]")
{
    const auto [first, second] = runRngCase(drawSeededStreams, &ParallelRngCases::streams);
    REQUIRE(first == second);
}

TEST_CASE("integer GetRandomNumber honors the half-open contract", "[rng]")
{
    xirand::rng().seed(1);

    bool sawMin = false;
    bool sawTop = false; // max - 1, the highest reachable value
    for (int i = 0; i < kSamples; ++i)
    {
        const auto value = xirand::GetRandomNumber<int32_t>(-3, 3);
        REQUIRE(value >= -3);
        REQUIRE(value < 3);
        sawMin = sawMin || value == -3;
        sawTop = sawTop || value == 2;
    }
    CHECK(sawMin);
    CHECK(sawTop);

    // Empty and inverted intervals collapse to min, mirroring math.random(n, n).
    CHECK(xirand::GetRandomNumber<int32_t>(4, 4) == 4);
    CHECK(xirand::GetRandomNumber<int32_t>(7, 2) == 7);
}

TEST_CASE("floating GetRandomNumber stays inside [min, max)", "[rng]")
{
    xirand::rng().seed(1);

    // Doubles over a sub-integer span (the self-destruct mobskill multiplier range).
    double minSeen = std::numeric_limits<double>::max();
    double maxSeen = std::numeric_limits<double>::lowest();
    for (int i = 0; i < kSamples; ++i)
    {
        const auto value = xirand::GetRandomNumber<double>(0.7, 1.1);
        REQUIRE(value >= 0.7);
        REQUIRE(value < 1.1);
        minSeen = std::min(minSeen, value);
        maxSeen = std::max(maxSeen, value);
    }
    CHECK(maxSeen > minSeen); // varies rather than collapsing to a constant

    // Narrowing to float is where a scaled draw could round up onto the excluded
    // max; scaleCanonical guards that edge and this hammers it.
    for (int i = 0; i < kSamples; ++i)
    {
        const auto value = xirand::GetRandomNumber<float>(0.0f, 1.0f);
        REQUIRE(value >= 0.0f);
        REQUIRE(value < 1.0f);
    }
}

TEST_CASE(kDistribution, "[rng]")
{
    const auto [mean, stddev] = runRngCase(sampleNormalDistribution, &ParallelRngCases::distribution);

    CHECK(std::abs(mean - 3.5) < 0.05);
    CHECK(std::abs(stddev - 1.5) < 0.05);
}

TEST_CASE("GetNormalNumber truncation and degenerate inputs", "[rng]")
{
    xirand::rng().seed(1);

    // Both truncation bounds respected, with real spread inside them.
    double minSeen = std::numeric_limits<double>::max();
    double maxSeen = std::numeric_limits<double>::lowest();
    for (int i = 0; i < kSamples; ++i)
    {
        const auto value = xirand::GetNormalNumber(3.5, 1.5, 2.0, 7.0);
        REQUIRE(value >= 2.0);
        REQUIRE(value <= 7.0);
        minSeen = std::min(minSeen, value);
        maxSeen = std::max(maxSeen, value);
    }
    CHECK(maxSeen > minSeen);

    constexpr double inf = std::numeric_limits<double>::infinity();

    // One-sided bounds.
    for (int i = 0; i < 1'000; ++i)
    {
        CHECK(xirand::GetNormalNumber(3.5, 1.5, 0.0, inf) >= 0.0);
        CHECK(xirand::GetNormalNumber(3.5, 1.5, -inf, 4.0) <= 4.0);
    }

    // Zero stddev collapses to the mean, clamped into the bounds.
    CHECK(xirand::GetNormalNumber(5.0, 0.0) == 5.0);
    CHECK(xirand::GetNormalNumber(5.0, 0.0, -inf, 3.0) == 3.0);
    CHECK(xirand::GetNormalNumber(5.0, 0.0, 7.0, inf) == 7.0);

    // An empty interval returns the lower bound.
    CHECK(xirand::GetNormalNumber(5.0, 1.5, 7.0, 2.0) == 7.0);
}

TEST_CASE("GetWeightedIndex respects the weights", "[rng]")
{
    xirand::rng().seed(1);

    // { 70, 20, 10 } -> expect roughly those percentages.
    std::array<int, 3> counts{};
    for (int i = 0; i < kSamples; ++i)
    {
        const auto index = xirand::GetWeightedIndex({ 70.0, 20.0, 10.0 });
        REQUIRE(index < counts.size());
        ++counts[index];
    }
    CHECK(std::abs(counts[0] / double(kSamples) - 0.70) < 0.02);
    CHECK(std::abs(counts[1] / double(kSamples) - 0.20) < 0.02);
    CHECK(std::abs(counts[2] / double(kSamples) - 0.10) < 0.02);

    // A zero weight is never selected; a single weight always is.
    for (int i = 0; i < 1'000; ++i)
    {
        CHECK(xirand::GetWeightedIndex({ 0.0, 1.0 }) == 1);
        CHECK(xirand::GetWeightedIndex({ 5.0 }) == 0);
    }
}

TEST_CASE("GetRandomElement and GetWeightedElement pick members", "[rng]")
{
    xirand::rng().seed(1);

    const std::vector<int> pool = { 2, 4, 8, 16, 32 };
    for (int i = 0; i < 1'000; ++i)
    {
        const auto element = xirand::GetRandomElement(pool);
        CHECK(std::ranges::find(pool, element) != pool.end());
    }

    const std::vector<int> single = { 42 };
    CHECK(xirand::GetRandomElement(single) == 42);

    // Weighted keys: the zero-weighted entry must never come back.
    const std::map<std::string, double> table = {
        { "Common", 70.0 },
        { "Rare", 30.0 },
        { "Impossible", 0.0 },
    };
    for (int i = 0; i < 1'000; ++i)
    {
        const auto key = xirand::GetWeightedElement(table);
        CHECK(key != "Impossible");
        CHECK(table.contains(key));
    }
}

TEST_CASE("ShuffleInPlace permutes without losing elements", "[rng]")
{
    std::vector<int> original(64);
    for (int i = 0; i < 64; ++i)
    {
        original[i] = i;
    }

    // Same seed -> same permutation; the multiset always survives.
    auto first = original;
    xirand::rng().seed(7);
    xirand::ShuffleInPlace(first);

    auto second = original;
    xirand::rng().seed(7);
    xirand::ShuffleInPlace(second);

    REQUIRE(first == second);
    CHECK(std::is_permutation(first.begin(), first.end(), original.begin()));
    CHECK(first != original); // 64 elements: an identity shuffle means something broke

    // Degenerate sizes are safe no-ops.
    std::vector<int> empty;
    xirand::ShuffleInPlace(empty);
    CHECK(empty.empty());

    std::vector<int> one = { 99 };
    xirand::ShuffleInPlace(one);
    CHECK(one == std::vector<int>{ 99 });
}

} // namespace
