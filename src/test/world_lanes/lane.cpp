#include "api.h"

#include "common/database.h"
#include "common/lua.h"
#include "common/settings.h"
#include "common/xi.h"
#include "map/ipc_client.h"
#include "map/map_networking.h"
#include "map/packets/c2s/0x0b6_chat_name.h"
#include "map/packets/s2c/0x009_message.h"
#include "test/lua/helpers/lua_client_entity_pair_packets.h"
#include "test/lua/lua_simulation.h"
#include "test/mock_socket.h"
#include "test/test_char.h"
#include "test/test_engine.h"

#include <cstdio>
#include <filesystem>
#include <unistd.h>

namespace
{

std::string          schema;
unsigned             fixtureCount{};
bool                 fixtureRoutesValid{ true };
const WorldMapInput* activeInput{};
uint32_t             heldCharacter{};

CLuaClientEntityPair* heldPlayer{};

int probeTell(int sendTell) noexcept
{
    try
    {
        if (!heldPlayer || !heldPlayer->testChar()->entity())
        {
            return -1;
        }
        if (sendTell)
        {
            auto  packets           = heldPlayer->packets();
            auto  packet            = packets.createPacket<GP_CLI_COMMAND_CHAT_NAME>();
            auto* tell              = packet->as<GP_CLI_COMMAND_CHAT_NAME>();
            tell->unknown04         = 3;
            tell->unknown05         = 0;
            constexpr char absent[] = "Absentworld";
            constexpr char text[]   = "fixture route";
            std::memcpy(tell->sName, absent, sizeof(absent));
            std::memcpy(tell->Mes, text, sizeof(text));
            packets.sendBasicPacket(*packet);
        }
        const auto& queue = heldPlayer->testChar()->entity()->getPacketList();
        return static_cast<int>(std::ranges::count_if(queue, [](const auto& packet)
                                                      {
                                                          if (packet->getType() != std::to_underlying(PacketS2C::GP_SERV_COMMAND_MESSAGE))
                                                          {
                                                              return false;
                                                          }
                                                          using Data         = GP_SERV_COMMAND_MESSAGE::PacketData;
                                                          const auto id      = packet->template ref<uint32_t>(sizeof(GP_SERV_HEADER) + offsetof(Data, UniqueNo));
                                                          const auto message = packet->template ref<uint16_t>(sizeof(GP_SERV_HEADER) + offsetof(Data, MesNo));
                                                          return id == heldCharacter && message == std::to_underlying(MsgStd::TellNotReceivedOffline);
                                                      }));
    }
    catch (...)
    {
        return -1;
    }
}

int lookupRecipient(const char* name) noexcept
{
    try
    {
        const auto row = db::preparedStmt("SELECT DATABASE() AS db_name, EXISTS(SELECT 1 FROM accounts_sessions JOIN chars USING(charid) WHERE charname = ?) AS online", std::string(name));
        if (row && row->next() && row->get<std::string>("db_name") == schema)
        {
            return row->get<int>("online");
        }
    }
    catch (...)
    {
    }
    return -1;
}

int receiveWorld(const uint8_t* bytes, size_t size) noexcept
{
    try
    {
        std::vector<uint8_t> packet(bytes, bytes + size);
        message::detail::client().handleMessage(IPP{}, packet);
        return 1;
    }
    catch (...)
    {
        return 0;
    }
}

int verifyFixture() noexcept
{
    try
    {
        const auto row = db::preparedStmt("SELECT DATABASE() AS db_name, value FROM char_vars WHERE charid = ? AND varname = ?",
                                          heldCharacter,
                                          "WORLD_MAP_HOLD");
        return row && row->next() && row->get<std::string>("db_name") == schema && row->get<unsigned>("value") == activeInput->lane + 1;
    }
    catch (...)
    {
        return 0;
    }
}

void checkFixture(CLuaClientEntityPair& player)
{
    const auto account   = player.testChar()->accountId();
    const auto character = player.testChar()->charId();
    if (fixtureCount++ != 0)
    {
        return;
    }
    const auto row     = db::preparedStmt("SELECT DATABASE() AS db_name, CONNECTION_ID() AS connection_id, accid FROM chars WHERE charid = ?", character);
    fixtureRoutesValid = row && row->next() && row->get<std::string>("db_name") == schema && row->get<uint32>("accid") == account;
    if (fixtureRoutesValid)
    {
        std::fprintf(stderr, "lane=%u main_tid=%d sql_connection=%u\n", activeInput->lane, gettid(), row->get<uint32>("connection_id"));
    }
    std::fprintf(stderr, "pid=%d schema=%s account=%u character=%u route=%d vm=%p zones=%p backend=%p\n", getpid(), schema.c_str(), account, character, fixtureRoutesValid, static_cast<void*>(lua.lua_state()), static_cast<void*>(&g_PZoneList), static_cast<void*>(&db::getDatabase()));
    if (fixtureRoutesValid && (activeInput->fixtureCheckpoint || activeInput->checkRoute))
    {
        heldPlayer               = &player;
        heldCharacter            = character;
        const auto releasePlayer = xi::finally([]()
                                               {
                                                   heldPlayer = nullptr;
                                               });
        if (!activeInput->fixtureCheckpoint)
        {
            const auto before  = probeTell(0);
            fixtureRoutesValid = before >= 0 && probeTell(1) == before + 1;
            return;
        }
        const auto insert  = db::preparedStmt("INSERT INTO char_vars (charid, varname, value) VALUES (?, ?, ?)", character, "WORLD_MAP_HOLD", activeInput->lane + 1);
        fixtureRoutesValid = insert && activeInput->fixtureCheckpoint(activeInput->lane, account, character, verifyFixture, probeTell) && verifyFixture();
    }
}

int run(const WorldMapInput& input)
{
    if (input.lane > 1 || !input.login || !input.password || !input.schema ||
        std::string_view(input.schema) != (input.lane ? "xidb_world_lanes_peer" : "xidb_world_lanes"))
    {
        throw std::runtime_error("missing private map database configuration");
    }
    const std::filesystem::path directory(input.directory);
    const auto                  meshes = (directory / "meshes").string();
    std::filesystem::create_directories(meshes);
    logging::InitializeLog("test", (directory / "server.log").string(), false);
    lua_init();
    settings::init();
    settings::set("test.NAVMESH_OUTPUT_DIR", meshes);
    lua["xi"]["settings"]["test"]["NAVMESH_OUTPUT_DIR"] = meshes;
    schema                                              = input.schema;
    activeInput                                         = &input;
    auto network                                        = lua["xi"]["settings"]["network"].get<sol::table>();
    for (const auto& [key, value] : { std::pair{ "SQL_LOGIN", input.login }, { "SQL_PASSWORD", input.password }, { "SQL_DATABASE", input.schema } })
    {
        settings::set(std::string("network.") + key, std::string(value));
        network[key] = std::string(value);
    }
    const auto route = db::preparedStmt("SELECT DATABASE() AS db_name");
    if (!route || !route->next() || route->get<std::string>("db_name") != schema)
    {
        throw std::runtime_error("unexpected map database before initialization");
    }

    const auto       owner = std::this_thread::get_id();
    std::atomic_uint sent{};
    std::atomic_bool routingFailed{};
    std::atomic_int  rejectedType{ -1 };
    const auto       send = [&](std::span<const uint8_t> bytes)
    {
        ++sent;
        if (std::this_thread::get_id() != owner || !input.dispatch(input.lane, bytes.data(), bytes.size(), lookupRecipient, receiveWorld))
        {
            routingFailed = true;
            rejectedType  = bytes.empty() ? 0 : bytes.front();
        }
    };
    Scheduler  scheduler(2);
    const auto checkWorkerRoute = [&]()
    {
        const auto row = db::preparedStmt("SELECT DATABASE() AS db_name, CONNECTION_ID() AS connection_id");
        if (!row || !row->next() || row->get<std::string>("db_name") != schema)
        {
            return false;
        }
        std::fprintf(stderr, "lane=%u worker_tid=%d sql_connection=%u schema=%s\n", input.lane, gettid(), row->get<uint32>("connection_id"), schema.c_str());
        return true;
    };
    if (!scheduler.blockOnMainThread(scheduler.spawnOnWorkerThread(checkWorkerRoute)))
    {
        throw std::runtime_error("unexpected worker database before map initialization");
    }

    bool                        hostRunning = true;
    auto                        hostTick    = scheduler.intervalOnMainThread(std::chrono::milliseconds(50), [&]()
                                                                             {
                                                       hostRunning = hostRunning && input.pump(input.lane);
                                                                             });
    MapConfig                   mapConfig{ .isTestServer = true, .lazyZones = true, .controlledWeather = true };
    auto                        map = std::make_unique<MapEngine>(scheduler, mapConfig, [&](MapNetworking& net)
                                                                  {
                                               return std::make_unique<IPCClient>(net, send);
                                                                  });
    std::unique_ptr<TestEngine> test;
    const auto                  stopMap = xi::finally([&]()
                                                      {
                                         scheduler.stop();
                                         CLuaSimulation::observeSpawns(nullptr);
                                                      });
    map->networking().setSocket(std::make_unique<MockSocket>());
    scheduler.blockOnMainThread(map->init());
    map->onInitialize();

    auto              sink = std::make_shared<InMemorySink>();
    const std::string file(input.file);
    const auto        pattern = "^" + file.substr(0, file.size() - 4) + "\\.lua$";
    TestConfig        config{
        .loggerSink = sink, .verbose = input.verbose != 0, .output = (directory / "ctrf.json").string(), .keepGoing = input.keepGoing != 0, .retryCount = input.retries, .seed = input.seed, .filters = { .includePatterns = { pattern } }
    };
    config.takeInfrastructureError = [&]() -> std::string
    {
        const auto type = rejectedType.exchange(-1);
        return type < 0 ? std::string{} : fmt::format("Unsupported world request from {}: {}", file, ipc::toString(static_cast<ipc::MessageType>(type)));
    };
    CLuaSimulation::observeSpawns(checkFixture);
    bool success = false;
    {
        test = std::make_unique<TestEngine>(std::move(config), std::move(map));
        InMemorySink::capture(sink);
        success = test->collectedTests() > 0 && scheduler.blockOnMainThread(test->executeTests());
        scheduler.stop();
        test.reset();
    }
    CLuaSimulation::observeSpawns(nullptr);
    std::fprintf(stderr, "pid=%d lane=%u fixtures=%u world_requests=%u\n", getpid(), input.lane, fixtureCount, sent.load());
    return success && hostRunning && !routingFailed.load() && fixtureRoutesValid && std::filesystem::is_regular_file(directory / "ctrf.json") ? 0 : 1;
}

} // namespace

extern "C" int xi_run_world_map_v2(const WorldMapInput* input)
{
    int status = 2;
    try
    {
        status = run(*input);
    }
    catch (const std::exception& error)
    {
        std::fprintf(stderr, "world map: %s\n", error.what());
    }
    catch (...)
    {
        std::fprintf(stderr, "world map: unknown exception\n");
    }
    activeInput = nullptr;
    try
    {
        lua_cleanup();
        logging::ShutDown();
    }
    catch (...)
    {
        return 2;
    }
    return status;
}
