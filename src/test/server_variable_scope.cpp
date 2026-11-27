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

#include <stdexcept>

auto ServerVariableScope::readRows() -> Rows
{
    const auto result = db::preparedStmt("SELECT name, value, expiry FROM server_variables");
    if (!result)
    {
        throw std::runtime_error("Could not read server_variables");
    }

    Rows rows;
    while (result->next())
    {
        rows.try_emplace(result->get<std::string>("name"), result->get<int32>("value"), result->get<int32>("expiry"));
    }
    return rows;
}

ServerVariableScope::ServerVariableScope()
: rows_(readRows())
{
}

ServerVariableScope::~ServerVariableScope()
{
    try
    {
        restoreRows();
    }
    catch (const std::exception& err)
    {
        ShowCriticalFmt("Could not restore server variable fixture: {}", err.what());
        std::terminate();
    }
}

void ServerVariableScope::restoreRows() const
{
    const auto current = readRows();
    if (current == rows_)
    {
        return;
    }

    const auto restore = [&]
    {
        for (const auto& [name, row] : current)
        {
            if (!rows_.contains(name) && !db::preparedStmt("DELETE FROM server_variables WHERE name = ?", name))
            {
                throw std::runtime_error(fmt::format("Could not remove server variable {}", name));
            }
        }

        for (const auto& [name, row] : rows_)
        {
            if (const auto it = current.find(name); it != current.end() && it->second == row)
            {
                continue;
            }

            if (!db::preparedStmt("INSERT INTO server_variables (name, value, expiry) VALUES (?, ?, ?) "
                                  "ON DUPLICATE KEY UPDATE value = VALUES(value), expiry = VALUES(expiry)",
                                  name,
                                  row.first,
                                  row.second))
            {
                throw std::runtime_error(fmt::format("Could not restore server variable {}", name));
            }
        }
    };

    if (!db::transaction(restore))
    {
        throw std::runtime_error("Could not commit server variable restoration");
    }
}
