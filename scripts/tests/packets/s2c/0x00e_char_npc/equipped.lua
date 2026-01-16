-- Equipped (SubKind 1) packet tests
-- Tests for NPCs with equipment slots (gear visible)
-- Retail captures from NPCPacketLog/Nours/npc_packets_Aht_Urhgan_Whitegate.lua
--
-- Rytaal (16982171) - Assault NPC in Whitegate
-- Position: x=112.002, y=-45.038, z=-0.338

---@type CharNpcTestSuite
return
{
    ['Whitegate Rytaal'] =
    {
        test = function(player)
            player:gotoZone(xi.zone.AHT_URHGAN_WHITEGATE)
            -- Move near Rytaal
            player.entities:moveTo('Rytaal')
            xi.test.world:tick()
        end,
        expected =
        {
            -- Rytaal spawn packet
            {
                UniqueNo      = 16982171,
                SendFlg       =
                {
                    ClaimStatus = 1,
                    General     = 1,
                    Model       = 1,
                    Name2       = 1,
                    Position    = 1,
                },
                Speed         = 40, -- Retail 50
                SpeedBase     = 40, -- Retail 50
                Hpp           = 100,
                Flags1        =
                {
                    AnonymousFlag  = 1,
                    CliPosInitFlag = 1,
                    GraphSize      = 1,
                    LfgFlag        = 1,
                },
                Flags2        =
                {
                    NamedFlag = 1,
                },
                Flags3        =
                {
                    MonStat = 1,
                },
                SubKind       = 1,
                Data          =
                {
                    GrapIDTbl =
                    {
                        264,
                        4160,
                        8351,
                        12447,
                        16543,
                        20639,
                        24576,
                        28672,
                        0,
                    },
                },
            },
        },
    },
}
