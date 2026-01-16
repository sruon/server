-- FixedModel (SubKind 0) packet tests
-- Used by mobs, pets, trusts, simple NPCs

---@type CharNpcTestSuite
return
{
    ['Mob spawn sends FixedModel packet'] =
    {
        test = function(player)
            player.entities:moveTo(17293511)
        end,
        expected =
        {
            UniqueNo = 17293511,
            ActIndex = 199,
            SendFlg =
            {
                ClaimStatus = 1,
                General     = 1,
                Name        = 1,
                Position    = 1,
            },
            Speed = 40,
            SpeedBase = 40,
            Hpp = 100,
            Flags1 =
            {
                CliPosInitFlag = 1,
                Gender         = 1,
                GraphSize      = 1,
                MonsterFlag    = 1,
            },
            Flags2 =
            {
                g = 13,
            },
        },
    },

    ['Ethereal Junction'] =
    {
        test = function(player)
            player.entities:moveTo(17293792)
        end,
        expected =
        {
            UniqueNo  = 17293792,
            ActIndex  = 480,
            SendFlg   =
            {
                ClaimStatus = 1,
                General     = 1,
                Name        = 1,
                Position    = 1,
            },
            Flags0    =
            {
                GroundFlag = 1,
            },
            Speed     = 50,
            SpeedBase = 50,
            Hpp       = 100,
            Flags1    =
            {
                CliPosInitFlag = 1,
                GraphSize = 1,
            },
            Flags3    =
            {
                CliPriorityFlag = 1,
                MonStat         = 5,
                unknown_3_2     = 1,
                unknown_3_4     = 1,
                unknown_3_5     = 1,
                unknown_3_6     = 1,
            },
        },
    },

    ['Land Worm roaming'] =
    {
        test = function(player)
            local worm = player.entities:moveTo(17293413)
            worm:despawn()
            worm:setMobMod(xi.mobMod.ROAM_COOL, 9999)
            player.packets:clear()
            worm:spawn()
            worm:resetRoamTimer()
            player.entities:moveTo(17293413)
            xi.test.world:tick()

            -- Trigger burrow then hidden (2s delay)
            worm:setMobMod(xi.mobMod.ROAM_COOL, 0)
            for i = 1, 4 do
                xi.test.world:skipTime(1)
                xi.test.world:tickEntity(worm)
                xi.test.world:tick()
            end
        end,
        -- Retail worm 17293413 state transitions from npc_packets_Qufim_Island.lua
        expected =
        {
            -- State 1: Spawned above ground (sync 6452)
            {
                UniqueNo = 17293413,
                ActIndex = 101,
                SendFlg =
                {
                    ClaimStatus = 1,
                    General     = 1,
                    Position    = 1,
                },
                Speed = 40,
                SpeedBase = 40,
                Hpp = 100,
                Flags1 =
                {
                    CliPosInitFlag = 1,
                    Gender         = 1,
                    GraphSize      = 1,
                    MonsterFlag    = 1,
                },
                Flags2 =
                {
                    g = 12,
                },
            },
            -- State 2: Burrowing animation (sync 6456)
            {
                UniqueNo = 17293413,
                ActIndex = 101,
                SendFlg =
                {
                    ClaimStatus = 1,
                    General     = 1,
                    Position    = 1,
                },
                Speed = 40,
                SpeedBase = 40,
                Hpp = 100,
                Flags1 =
                {
                    CliPosInitFlag = 1,
                    Gender         = 1,
                    GraphSize      = 1,
                    MonsterFlag    = 1,
                },
                Flags2 =
                {
                    g = 12,
                },
                Flags3 =
                {
                    MonStat = 1,
                },
            },
            -- State 3: Fully hidden underground (sync 6464)
            {
                UniqueNo = 17293413,
                ActIndex = 101,
                SendFlg =
                {
                    ClaimStatus = 1,
                    General     = 1,
                    Position    = 1,
                },
                Speed = 40,
                SpeedBase = 40,
                Hpp = 100,
                Flags1 =
                {
                    CliPosInitFlag = 1,
                    Gender         = 1,
                    GraphSize      = 1,
                    HideFlag       = 1,
                    MonsterFlag    = 1,
                },
                Flags2 =
                {
                    g = 12,
                },
                Flags3 =
                {
                    MonStat = 1,
                },
            },
        },
    },

    ['Antlion ambush aggro'] =
    {
        test = function(player)
            player:gotoZone(xi.zone.ATTOHWA_CHASM)
            local antlion = player.entities:moveTo(16806238) -- Cave Antlion
            player.actions:move(antlion:getXPos(), antlion:getYPos(), antlion:getZPos() + 2)
            for i = 1, 10 do
                xi.test.world:tickEntity(antlion)
                xi.test.world:skipTime(1)
                if antlion:isEngaged() then
                    break
                end
            end
        end,
        expected =
        {
            -- Hidden underground (retail sync 108)
            {
                UniqueNo = 16806238,
                ActIndex = 350,
                SendFlg =
                {
                    ClaimStatus = 1,
                    General     = 1,
                    Name        = 1,
                    Position    = 1,
                },
                Speed = 40,
                SpeedBase = 40,
                Hpp = 100,
                Flags1 =
                {
                    CliPosInitFlag = 1,
                    Gender         = 1,
                    GraphSize      = 2,
                    MonsterFlag    = 1,
                    TargetOffFlag  = 1,
                },
                Flags2 =
                {
                    g = 13,
                },
                Flags3 =
                {
                    MonStat     = 4,
                    unknown_3_3 = 1,
                    unknown_3_4 = 1,
                },
            },
            -- Emerged and engaged (retail sync 229)
            {
                UniqueNo = 16806238,
                ActIndex = 350,
                SendFlg =
                {
                    ClaimStatus = 1,
                    General     = 1,
                    Position    = 1,
                },
                Speed = 40,
                SpeedBase = 40,
                Hpp = 100,
                server_status = 1,
                Flags1 =
                {
                    CliPosInitFlag = 1,
                    Gender         = 1,
                    GraphSize      = 2,
                    MonsterFlag    = 1,
                },
                Flags2 =
                {
                    g = 13,
                },
                Flags3 =
                {
                    MonStat = 5,
                },
            },
        },
    },
}
