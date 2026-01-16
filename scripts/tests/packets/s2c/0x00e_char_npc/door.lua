-- Door (SubKind 2) packet tests
-- Tests for interactive doors
-- Retail captures from NPCPacketLog/Nours/npc_packets_Aht_Urhgan_Whitegate.lua
--
-- Door: Chamber of Passage (16982068 / _1e4)
-- Position: x=111.975, z=-1.670, y=24.867
--
-- Door states:
--   8 = open
--   9 = closed

---@type CharNpcTestSuite
return
{
    ['Whitegate Chamber of Passage Door'] =
    {
        test = function(player)
            player:gotoZone(xi.zone.AHT_URHGAN_WHITEGATE)
            xi.test.world:tick()

            local door = player.entities:get(16982068)

            -- Position near door
            player.actions:move(111.975, 24.867, -3.0)
            xi.test.world:tick()

            -- Open door (7 second default)
            door:openDoor()
            xi.test.world:tick()

            -- Wait for door to close
            xi.test.world:skipTime(10)
            xi.test.world:tick()
        end,
        expected =
        {
            -- Door open
            {
                UniqueNo      = 16982068,
                SendFlg       =
                {
                    ClaimStatus = 1,
                    General     = 1,
                    Position    = 1,
                },
                Flags0        =
                {
                    MovTime = 1,
                },
                Speed         = 40, -- Retail is 50
                SpeedBase     = 40, -- Retail is 50
                Hpp           = 100,
                server_status = 8,
                Flags1        =
                {
                    CliPosInitFlag = 1,
                    GraphSize      = 1,
                    TalkUcoffFlag  = 1,
                },
                SubKind       = 2,
                Data          =
                {
                    DoorId = 879047007, -- "_1e4"
                },
            },
            -- Door closed
            {
                UniqueNo      = 16982068,
                SendFlg       =
                {
                    ClaimStatus = 1,
                    General     = 1,
                    Position    = 1,
                },
                Flags0        =
                {
                    MovTime = 1,
                },
                Speed         = 40, -- Retail is 50
                SpeedBase     = 40, -- Retail is 50
                Hpp           = 100,
                server_status = 9,
                Flags1        =
                {
                    CliPosInitFlag = 1,
                    GraphSize      = 1,
                    TalkUcoffFlag  = 1,
                },
                SubKind       = 2,
                Data          =
                {
                    DoorId = 879047007, -- "_1e4"
                },
            },
        },
    },
}
