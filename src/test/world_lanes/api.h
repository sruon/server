#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef int (*WorldMapVerify)(void);
    typedef int (*WorldMapProbe)(int sendTell);
    typedef int (*WorldMapLookup)(const char* name);
    typedef int (*WorldMapReceive)(const uint8_t* bytes, size_t size);

    struct WorldMapInput
    {
        const char* file;
        const char* directory;
        const char* schema;
        const char* login;
        const char* password;
        uint64_t    seed;
        unsigned    lane;
        size_t      retries;
        uint8_t     verbose;
        uint8_t     keepGoing;
        uint8_t     checkRoute;
        int (*pump)(unsigned lane);
        int (*fixtureCheckpoint)(unsigned lane, uint32_t account, uint32_t character, WorldMapVerify verify, WorldMapProbe probe);
        int (*dispatch)(unsigned lane, const uint8_t* bytes, size_t size, WorldMapLookup lookup, WorldMapReceive receive);
    };

    int xi_run_world_map_v2(const struct WorldMapInput* input);

#ifdef __cplusplus
}
#endif
