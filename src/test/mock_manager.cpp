/*
===========================================================================

  Copyright (c) 2025 LandSandBoat Dev Teams

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

#include "mock_manager.h"

#include "common/logging.h"
#include "lua/lua_spy.h"
#include "lua/lua_stub.h"
#include "map/lua/luautils.h"

#include <stdexcept>

MockManager::MockManager() = default;

MockManager::~MockManager()
{
    for (const auto& error : restoreAll())
    {
        ShowErrorFmt("{}", error);
    }
}

// Installs a stub in the lua global environment at the specified path.
// The path is a dot-separated string representing the table hierarchy.
auto MockManager::stub(const std::string& path, const sol::object& mockImpl) -> CLuaStub*
{
    DebugTestFmt("Installing stub at path: {}", path);

    auto        original = luautils::detail::findGlobalLuaFunction(path);
    const auto& stub     = stubs_.emplace_back(std::make_unique<CLuaStub>(path, original, mockImpl));
    setAtPath(path, sol::make_object(lua, stub.get()));

    return stub.get();
}

// Installs a spy in the lua global environment at the specified path.
// The path is a dot-separated string representing the table hierarchy.
auto MockManager::spy(const std::string& path) -> CLuaSpy*
{
    DebugTestFmt("Installing spy at path: {}", path);

    auto        original = luautils::detail::findGlobalLuaFunction(path);
    const auto& spy      = spies_.emplace_back(std::make_unique<CLuaSpy>(path, original));
    setAtPath(path, sol::make_object(lua, spy.get()));

    return spy.get();
}

// Restores all stubs and spies to their original functions.
auto MockManager::restoreAll() -> std::vector<std::string>
{
    auto       errors  = std::vector<std::string>{};
    const auto restore = [&](const auto& testDouble) -> bool
    {
        try
        {
            setAtPath(testDouble->path(), testDouble->original());
            return true;
        }
        catch (const std::exception& error)
        {
            errors.push_back(fmt::format("Restoring '{}' failed: {}", testDouble->path(), error.what()));
        }
        catch (...)
        {
            errors.push_back(fmt::format("Restoring '{}' failed: unknown exception", testDouble->path()));
        }

        return false;
    };

    auto failedStubs = std::vector<std::unique_ptr<CLuaStub>>{};
    auto failedSpies = std::vector<std::unique_ptr<CLuaSpy>>{};
    for (auto& stub : stubs_)
    {
        if (!restore(stub))
        {
            failedStubs.push_back(std::move(stub));
        }
    }

    for (auto& spy : spies_)
    {
        if (!restore(spy))
        {
            failedSpies.push_back(std::move(spy));
        }
    }

    stubs_ = std::move(failedStubs);
    spies_ = std::move(failedSpies);
    return errors;
}

// Helper function to set a value at a given path in the lua global environment.
void MockManager::setAtPath(const std::string& path, const sol::object& value) const
{
    auto parts = split(path, ".");
    if (parts.empty())
    {
        return;
    }

    sol::table current = lua.globals();

    for (size_t i = 0; i < parts.size() - 1; ++i)
    {
        const auto next = current[parts[i]].get<sol::optional<sol::table>>();
        if (!next.has_value())
        {
            throw std::runtime_error(fmt::format("'{}' is not a table in '{}'", parts[i], path));
        }

        current = next.value();
    }

    current[parts.back()] = value;
}
