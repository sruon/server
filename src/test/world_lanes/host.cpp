#include "host.h"
#include "api.h"

#include <cstdio>
#include <features.h>

#ifdef __GLIBC__

#include "common/arguments.h"
#include "common/database.h"
#include "common/settings.h"
#include "common/xi.h"
#include "map/zone.h"
#include "test/test_application.h"
#include "world/ipc_server.h"
#include "world/world_engine.h"

#include <charconv>
#include <cstdlib>
#include <dlfcn.h>
#include <filesystem>
#include <link.h>
#include <regex>
#include <signal.h>
#include <unistd.h>

namespace
{

WorldEngine*                                               hostWorld{};
unsigned                                                   activeLane{};
std::function<bool()>                                      runPeer;
WorldMapVerify                                             verifyPrimary{};
WorldMapProbe                                              probePrimary{};
WorldMapLookup                                             activeLookup{};
bool                                                       checkRoute{};
bool                                                       heldBoth{};
std::array<std::optional<ipc::Channel<zmq::message_t>>, 2> dealers;

IPP mapAddress(unsigned lane)
{
    return IPP(str2ip("127.0.0.1"), static_cast<uint16_t>(65001 + lane));
}

auto signalActions()
{
    constexpr std::array                                      signals{ SIGINT, SIGTERM, SIGSEGV, SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGPIPE, SIGXFSZ, SIGTRAP, SIGUSR2 };
    std::array<std::pair<void (*)(int), int>, signals.size()> actions{};
    for (size_t i = 0; i < signals.size(); ++i)
    {
        struct sigaction action{};
        if (sigaction(signals[i], nullptr, &action) != 0)
        {
            throw std::runtime_error("cannot inspect process signal dispositions");
        }
        actions[i] = { action.sa_handler, action.sa_flags };
    }
    return actions;
}

int fixtureCheckpoint(unsigned lane, uint32_t account, uint32_t character, WorldMapVerify verify, WorldMapProbe probe) noexcept
{
    if (lane != activeLane || account != 20000000 || character != 20000000 || !verify())
    {
        return 0;
    }
    try
    {
        if (lane == 0)
        {
            verifyPrimary      = verify;
            probePrimary       = probe;
            const auto success = runPeer();
            activeLane         = 0;
            verifyPrimary      = nullptr;
            return success && heldBoth && verify();
        }
        heldBoth = verifyPrimary && verifyPrimary() && verify();
        if (heldBoth && checkRoute)
        {
            const auto before0 = probePrimary(0);
            const auto before1 = probe(0);
            activeLane         = 0;
            const auto after0  = probePrimary(1);
            const auto other1  = probe(0);
            activeLane         = 1;
            const auto after1  = probe(1);
            const auto other0  = probePrimary(0);
            heldBoth           = before0 >= 0 && before1 >= 0 && after0 == before0 + 1 && after1 == before1 + 1 && other1 == before1 && other0 == after0;
            std::fprintf(stderr, "pid=%d fixture_id=%u outgoing_0x009_before=%d,%d after_primary=%d,%d after_peer=%d,%d isolated=%d\n", getpid(), character, before0, before1, after0, other1, other0, after1, heldBoth);
        }
        std::fprintf(stderr, "pid=%d held_both=%d character=%u primary_marker=1 peer_marker=2\n", getpid(), heldBoth, character);
        return heldBoth;
    }
    catch (...)
    {
        activeLane    = 0;
        verifyPrimary = nullptr;
        return 0;
    }
}

int pump(unsigned lane) noexcept
{
    try
    {
        if (!hostWorld || activeLane != lane)
        {
            return 0;
        }
        hostWorld->scheduler_.mainContext().poll();
        return !hostWorld->scheduler_.closeRequested();
    }
    catch (...)
    {
        return 0;
    }
}

int exchangeTell(unsigned lane, const uint8_t* bytes, size_t size, WorldMapLookup lookup, WorldMapReceive receive)
{
    if (activeLookup || !dealers[lane])
    {
        return 0;
    }
    activeLookup             = lookup;
    const auto releaseLookup = xi::finally([]()
                                           {
                                               activeLookup = nullptr;
                                           });
    dealers[lane]->send(zmq::message_t(bytes, size));
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    while (std::chrono::steady_clock::now() < deadline)
    {
        hostWorld->ipcServer_->handleIncomingMessages();
        for (unsigned target = 0; target < dealers.size(); ++target)
        {
            zmq::message_t reply;
            if (dealers[target] && dealers[target]->tryReceive(reply))
            {
                if (target != lane || !reply.size())
                {
                    std::fprintf(stderr, "world response reached the wrong endpoint\n");
                    return 0;
                }
                const auto type = ipc::toString(static_cast<ipc::MessageType>(*reply.data<uint8_t>()));
                std::fprintf(stderr, "pid=%d origin=%u dealer=%u request=ChatMessageTell response=%s\n", getpid(), lane, target, type.data());
                return receive(reply.data<uint8_t>(), reply.size());
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::fprintf(stderr, "world tell response timed out for map %u\n", lane);
    return 0;
}

int dispatch(unsigned lane, const uint8_t* bytes, size_t size, WorldMapLookup lookup, WorldMapReceive receive) noexcept
{
    try
    {
        if (!hostWorld || activeLane != lane || !bytes || size == 0)
        {
            return 0;
        }
        if (bytes[0] == static_cast<uint8_t>(ipc::EnumTypeV<ipc::ChatMessageTell>))
        {
            return exchangeTell(lane, bytes, size, lookup, receive);
        }
        const auto message = ipc::fromBytesWithHeader<ipc::CharZone>({ bytes, size });
        if (message && message->destinationZoneId == ZONE_NO_DESTINATION)
        {
            hostWorld->ipcServer_->handleMessage_CharZone(IPP{}, *message);
            return 1;
        }
        std::fprintf(stderr, "lane=%u unsupported world request=%s\n", lane, ipc::toString(static_cast<ipc::MessageType>(bytes[0])).data());
    }
    catch (...)
    {
    }
    return 0;
}

bool run(TestApplication& app, const char* module)
{
    const auto files  = app.args().get<std::vector<std::string>>("--file");
    const auto output = app.args().present<std::string>("--output");
    if (files.empty() || files.size() > 2 || !output)
    {
        throw std::runtime_error("world maps require one or two exact --file paths and an --output directory");
    }
    for (const auto& file : files)
    {
        if (!std::regex_match(file, std::regex("scripts/tests/([A-Za-z0-9_/-]+)\\.lua")) || !std::filesystem::is_regular_file(file))
        {
            throw std::runtime_error("invalid world map test file: " + file);
        }
    }

    uint64_t seed = 42;
    if (const auto arg = app.args().present<std::string>("--seed"))
    {
        const auto [last, err] = std::from_chars(arg->data(), arg->data() + arg->size(), seed);
        if (err != std::errc{} || last != arg->data() + arg->size())
        {
            throw std::runtime_error("invalid world map seed");
        }
    }

    size_t retries{};
    if (const auto arg = app.args().present<std::string>("--retry"))
    {
        const auto [last, err] = std::from_chars(arg->data(), arg->data() + arg->size(), retries);
        if (err != std::errc{} || last != arg->data() + arg->size())
        {
            throw std::runtime_error("invalid world map retry count");
        }
    }
    for (const auto* filter : { "--tag", "--no-tag", "--no-file", "--filter", "--no-filter" })
    {
        if (!app.args().get<std::vector<std::string>>(filter).empty())
        {
            throw std::runtime_error("world maps require whole files");
        }
    }
    if (app.args().get<bool>("--watch"))
    {
        throw std::runtime_error("world maps require a single pass");
    }

    const auto hostRoute = db::preparedStmt("SELECT DATABASE() AS db_name");
    if (!hostRoute || !hostRoute->next() || hostRoute->get<std::string>("db_name") != "xidb_world_lanes")
    {
        throw std::runtime_error("world host requires its private primary schema");
    }
    settings::set("network.ZMQ_TRANSPORT", std::string("inproc"));
    WorldEngine world(app.scheduler(), app.zmqService(), WorldEngine::EnableHTTPServer::No);
    const auto  stopHost = xi::finally([&]()
                                       {
                                          app.scheduler().stop();
                                          hostWorld     = nullptr;
                                          runPeer       = {};
                                          verifyPrimary = nullptr;
                                          probePrimary  = nullptr;
                                          for (auto& dealer : dealers)
                                          {
                                              dealer.reset();
                                          }
                                       });
    world.onInitialize();
    checkRoute          = std::getenv("XI_TEST_WORLD_CHECK_ROUTE") != nullptr;
    const auto endpoint = fmt::format("inproc://{}:{}", settings::get<std::string>("network.ZMQ_IP"), settings::get<uint16>("network.ZMQ_PORT"));
    for (unsigned lane = 0; lane < files.size(); ++lane)
    {
        dealers[lane].emplace(app.zmqService().registerDealer(endpoint, mapAddress(lane).getRawIPP()));
    }
    world.ipcServer_->setTellLookup([](const IPP& source, const std::string& name) -> Maybe<IPP>
                                    {
                                        if (!activeLookup || source != mapAddress(activeLane))
                                        {
                                            throw std::runtime_error("unregistered world tell source");
                                        }
                                        const int online = activeLookup(name.c_str());
                                        if (online < 0)
                                        {
                                            throw std::runtime_error("originating map could not look up tell recipient");
                                        }
                                        if (online)
                                        {
                                            return source;
                                        }
                                        return std::nullopt;
                                    });
    hostWorld              = &world;
    const bool holdFixture = std::getenv("XI_TEST_WORLD_HOLD_FIXTURE") != nullptr;
    if (holdFixture && files.size() != 2)
    {
        throw std::runtime_error("holding a fixture requires two map files");
    }

    const auto runLane = [&](unsigned lane)
    {
        const std::string library = std::string(module) + (lane ? ".peer" : "");
        if (lane && std::filesystem::equivalent(module, library))
        {
            throw std::runtime_error("map modules require distinct inodes");
        }
        const auto directory = (std::filesystem::path(*output) / ("map-" + std::to_string(lane))).string();
        std::filesystem::create_directories(directory);
        if (std::filesystem::exists(std::filesystem::path(directory) / "ctrf.json"))
        {
            throw std::runtime_error("existing map report in " + directory);
        }
        const auto signalsBefore   = signalActions();
        const auto terminateBefore = std::get_terminate();
        auto*      handle          = dlopen(library.c_str(), RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE);
        if (!handle)
        {
            throw std::runtime_error(dlerror());
        }
        Lmid_t namespaceId{};
        if (dlinfo(handle, RTLD_DI_LMID, &namespaceId) != 0 || namespaceId != LM_ID_BASE)
        {
            throw std::runtime_error("map module must share the base runtime");
        }
        auto entry = reinterpret_cast<decltype(&xi_run_world_map_v2)>(dlsym(handle, "xi_run_world_map_v2"));
        if (!entry)
        {
            throw std::runtime_error(dlerror());
        }
        const WorldMapInput input{
            files[lane].c_str(), directory.c_str(), std::getenv(lane ? "WORLD_PEER_DATABASE" : "XI_NETWORK_SQL_DATABASE"), std::getenv(lane ? "WORLD_PEER_LOGIN" : "XI_NETWORK_SQL_LOGIN"), std::getenv(lane ? "WORLD_PEER_PASSWORD" : "XI_NETWORK_SQL_PASSWORD"), seed, lane, retries, app.args().get<bool>("--verbose"), app.args().get<bool>("--keep-going"), checkRoute, pump, holdFixture ? fixtureCheckpoint : nullptr, dispatch
        };
        activeLane         = lane;
        const auto start   = std::chrono::steady_clock::now();
        const auto startNs = std::chrono::duration_cast<std::chrono::nanoseconds>(start.time_since_epoch()).count();
        std::fprintf(stderr, "pid=%d world=%p lane=%u file=%s lmid=%ld start_ns=%lld\n", getpid(), static_cast<void*>(&world), lane, input.file, static_cast<long>(namespaceId), static_cast<long long>(startNs));
        const int  status = entry(&input);
        const auto ms     = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
        std::fprintf(stderr, "pid=%d lane=%u file=%s status=%d elapsed_ms=%lld\n", getpid(), lane, input.file, status, static_cast<long long>(ms));
        dlclose(handle);
        if (signalsBefore != signalActions() || terminateBefore != std::get_terminate())
        {
            throw std::runtime_error("map execution changed a process signal or terminate handler");
        }
        return status == 0;
    };

    if (holdFixture)
    {
        runPeer = [&]()
        {
            return runLane(1);
        };
        const bool success = runLane(0);
        runPeer            = {};
        if (!heldBoth)
        {
            std::fprintf(stderr, "world maps did not verify both held fixtures\n");
        }
        return success && heldBoth;
    }

    const bool first = runLane(0);
    if (files.size() == 1 || (!first && !app.args().get<bool>("--keep-going")))
    {
        return first;
    }
    return runLane(1) && first;
}

} // namespace

bool runWorldLanes(TestApplication& app, const char* module)
{
    try
    {
        return run(app, module);
    }
    catch (const std::exception& error)
    {
        std::fprintf(stderr, "world maps: %s\n", error.what());
        hostWorld = nullptr;
        app.scheduler().stop();
        return false;
    }
}
#else
bool runWorldLanes(TestApplication&, const char*)
{
    std::fprintf(stderr, "World map research requires glibc.\n");
    return false;
}
#endif
