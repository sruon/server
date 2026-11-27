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

#include "test_char.h"

#include <common/database.h>
#include <common/xi.h>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <limits>

namespace
{

auto fixtureIdBase() -> uint32
{
    const auto rset = db::preparedStmt("SELECT GREATEST(COALESCE((SELECT MAX(id) FROM accounts), 0), "
                                       "COALESCE((SELECT MAX(charid) FROM chars), 0), 20000000) AS max_id");
    REQUIRE(rset);
    REQUIRE(rset->next());
    const auto maxId = rset->get<uint32>("max_id");
    REQUIRE(maxId < std::numeric_limits<uint32>::max() - 20);
    return maxId + 10;
}

auto countRows(const db::Query query, const uint32 id) -> uint64
{
    const auto rset = db::preparedStmt(db::selected(query), id);
    REQUIRE(rset);
    REQUIRE(rset->next());
    return rset->get<uint64>("count");
}

void removeFixtures(const uint32 firstId, const uint32 lastId)
{
    CHECK(db::preparedStmt("DELETE FROM accounts WHERE id BETWEEN ? AND ?", firstId, lastId));
    CHECK(db::preparedStmt("DELETE FROM chars WHERE charid BETWEEN ? AND ?", firstId, lastId));
    CHECK(db::preparedStmt("DELETE FROM char_stats WHERE charid BETWEEN ? AND ?", firstId, lastId));
    CHECK(db::preparedStmt("DELETE FROM char_flags WHERE charid BETWEEN ? AND ?", firstId, lastId));
}

} // namespace

TEST_CASE("TestChar teardown uses its account ID independently of its character ID", "[database][test_char]")
{
    const auto base    = fixtureIdBase();
    const auto cleanup = xi::finally([&]
                                     {
                                         removeFixtures(base, base + 6);
                                     });

    REQUIRE(db::preparedStmt("INSERT INTO accounts (id, login) VALUES (?, 'fixture_other'), (?, 'fixture_anchor')", base + 1, base + 5));
    REQUIRE(db::preparedStmt("INSERT INTO chars (charid, accid, charname, pos_zone) VALUES (?, ?, 'FixtureAnchor', ?)", base, base + 5, xi::ZoneId::GmHome));

    auto fixture = TestChar::create();
    REQUIRE(fixture);
    REQUIRE(fixture->accountId() == base + 6);
    REQUIRE(fixture->charId() == base + 1);
    fixture.reset();

    CHECK(countRows("SELECT COUNT(*) AS count FROM accounts WHERE id = ?", base + 6) == 0);
    CHECK(countRows("SELECT COUNT(*) AS count FROM chars WHERE charid = ?", base + 1) == 0);
    CHECK(countRows("SELECT COUNT(*) AS count FROM char_flags WHERE charid = ?", base + 1) == 0);
    CHECK(countRows("SELECT COUNT(*) AS count FROM accounts WHERE id = ?", base + 1) == 1);
    CHECK(countRows("SELECT COUNT(*) AS count FROM accounts WHERE id = ?", base + 5) == 1);
    CHECK(countRows("SELECT COUNT(*) AS count FROM chars WHERE charid = ?", base) == 1);
}

TEST_CASE("TestChar creation rolls back when saving a character fails", "[database][test_char]")
{
    const auto base    = fixtureIdBase();
    const auto cleanup = xi::finally([&]
                                     {
                                         removeFixtures(base, base + 1);
                                     });

    REQUIRE(db::preparedStmt("INSERT INTO accounts (id, login) VALUES (?, 'fixture_anchor')", base));
    REQUIRE(db::preparedStmt("INSERT INTO chars (charid, accid, charname, pos_zone) VALUES (?, ?, 'FixtureAnchor', ?)", base, base, xi::ZoneId::GmHome));
    REQUIRE(db::preparedStmt("INSERT INTO char_stats (charid, mjob) VALUES (?, ?)", base + 1, xi::Job::MNK));

    auto fixture = TestChar::create();
    CHECK_FALSE(fixture);
    CHECK_FALSE(db::getDatabase().isInTransaction());

    constexpr auto queries = std::to_array<db::Query>({
        "SELECT COUNT(*) AS count FROM accounts WHERE id = ?",
        "SELECT COUNT(*) AS count FROM chars WHERE charid = ?",
        "SELECT COUNT(*) AS count FROM char_look WHERE charid = ?",
        "SELECT COUNT(*) AS count FROM char_equip WHERE charid = ?",
        "SELECT COUNT(*) AS count FROM char_exp WHERE charid = ?",
        "SELECT COUNT(*) AS count FROM char_history WHERE charid = ?",
        "SELECT COUNT(*) AS count FROM char_inventory WHERE charid = ?",
        "SELECT COUNT(*) AS count FROM char_jobs WHERE charid = ?",
        "SELECT COUNT(*) AS count FROM char_pet WHERE charid = ?",
        "SELECT COUNT(*) AS count FROM char_points WHERE charid = ?",
        "SELECT COUNT(*) AS count FROM char_profile WHERE charid = ?",
        "SELECT COUNT(*) AS count FROM char_storage WHERE charid = ?",
        "SELECT COUNT(*) AS count FROM char_unlocks WHERE charid = ?",
        "SELECT COUNT(*) AS count FROM char_flags WHERE charid = ?",
    });

    for (const auto query : queries)
    {
        INFO(query.view());
        CHECK(countRows(query, base + 1) == 0);
    }

    CHECK(countRows("SELECT COUNT(*) AS count FROM char_stats WHERE charid = ?", base + 1) == 1);
    CHECK(countRows("SELECT COUNT(*) AS count FROM accounts WHERE id = ?", base) == 1);
    CHECK(countRows("SELECT COUNT(*) AS count FROM chars WHERE charid = ?", base) == 1);
}
