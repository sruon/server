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

#include "server_variable_scope.h"

#include "common/database.h"
#include "map/utils/serverutils.h"

#include <catch2/catch_test_macros.hpp>

namespace
{

auto readRow(const std::string& name) -> std::optional<std::pair<int32, int32>>
{
    const auto result = db::preparedStmt("SELECT value, expiry FROM server_variables WHERE name = ?", name);
    REQUIRE(result);
    if (!result->next())
    {
        return std::nullopt;
    }

    return std::pair{ result->get<int32>("value"), result->get<int32>("expiry") };
}

} // namespace

TEST_CASE("Server variable scopes restore rows, cached reads and pending flushes", "[database]")
{
    const ServerVariableScope outer;
    const std::string         dirty   = "[xi_test]scopeDirty";
    const std::string         clean   = "[xi_test]scopeClean";
    const std::string         cold    = "[xi_test]scopeCold";
    const std::string         absent  = "[xi_test]scopeAbsent";
    const std::string         expired = "[xi_test]scopeExpired";
    const std::string         zero    = "[xi_test]scopeZero";

    serverutils::SetServerVar(dirty, -7, 101);
    serverutils::SetVolatileServerVar(dirty, -8, 102);
    serverutils::SetServerVar(clean, 3);
    REQUIRE(serverutils::GetServerVar(clean) == 3);
    serverutils::SetServerVar(clean, 6);
    serverutils::SetServerVar(cold, 9);
    serverutils::SetServerVar(absent, 0);
    serverutils::SetServerVar(expired, 81, 1);
    REQUIRE(db::preparedStmt("REPLACE INTO server_variables (name, value, expiry) VALUES (?, 0, 12)", zero));

    {
        const ServerVariableScope inner;
        REQUIRE(serverutils::GetServerVar(expired) == 0);
        REQUIRE_FALSE(readRow(expired));
        serverutils::SetServerVar(cold, 0);
        REQUIRE(serverutils::GetVolatileServerVar(cold) == 0);
        serverutils::SetServerVar(zero, 0);
        serverutils::SetVolatileServerVar(clean, 40);
        serverutils::PersistVolatileServerVars();
        REQUIRE(readRow(dirty) == std::pair{ -8, 102 });
        serverutils::SetVolatileServerVar(absent, 14);
        serverutils::SetServerVar(absent, 15);
    }

    CHECK(readRow(dirty) == std::pair{ -7, 101 });
    CHECK(readRow(clean) == std::pair{ 6, 0 });
    CHECK(readRow(cold) == std::pair{ 9, 0 });
    CHECK(readRow(expired) == std::pair{ 81, 1 });
    CHECK(readRow(zero) == std::pair{ 0, 12 });
    CHECK_FALSE(readRow(absent));

    CHECK(serverutils::GetVolatileServerVar(clean) == 3);
    serverutils::SetServerVar(cold, 27);
    CHECK(serverutils::GetVolatileServerVar(cold) == 27);
    serverutils::PersistVolatileServerVars();
    CHECK(readRow(dirty) == std::pair{ -8, 102 });
    CHECK(readRow(clean) == std::pair{ 6, 0 });
    CHECK_FALSE(readRow(absent));
}
