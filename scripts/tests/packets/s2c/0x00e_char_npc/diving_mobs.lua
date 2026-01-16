-- Diving Mobs (SubKind 0) packet tests
-- Tests for mobs that dive/surface: Hpemde, Yovra, Greater Amphiptere
-- Retail captures from NPCPacketLog/Richies
-- NOTE: These tests use ONLY actual retail captured packet states

---@type CharNpcTestSuite
return
{
    ['Yovra'] =
    {
        test = function(player)
            player:gotoZone(xi.zone.ALTAIEU)
            local yovra = player.entities:get(16912624) -- Ul'yovra
            -- Move player 20y away so it doesn't aggro
            player.actions:move(yovra:getXPos() + 20, yovra:getYPos(), yovra:getZPos())
            xi.test.world:tick()
            -- Move player right below to aggro - triggers descent phase
            player.actions:move(yovra:getXPos(), yovra:getYPos(), yovra:getZPos())
            xi.test.world:tick()
            -- Move player away and disengage the Yovra, wait for it to float
            player.actions:move(yovra:getXPos() + 20, yovra:getYPos(), yovra:getZPos())
            yovra:disengage()
            xi.test.world:skipTime(15)
            xi.test.world:tick()
        end,
        expected =
        {
            {
                UniqueNo  = 16912624,
                ActIndex  = 240,
                SendFlg   =
                {
                    ClaimStatus = 1,
                    General     = 1,
                    Name        = 1,
                    Position    = 1,
                },
                Speed     = 40,
                SpeedBase = 40,
                Hpp       = 100,
                Flags1    =
                {
                    CliPosInitFlag = 1,
                    Gender         = 1,
                    GraphSize      = 1,
                    LinkDeadFlag   = 1,
                    LinkShellFlag  = 1,
                    MonsterFlag    = 1,
                    TargetOffFlag  = 1,
                },
                Flags2    =
                {
                    g = 12,
                },
                Flags3    =
                {
                    MonStat     = 5,
                    unknown_2_3 = 1,
                    unknown_3_3 = 1,
                    unknown_3_4 = 1,
                },
                Data      =
                {
                    Name = 'Ulyovra', -- Fixme: Should be Ul'Yovra
                    model_id = 1148,
                },
            },
            { -- Descending
                UniqueNo      = 16912624,
                ActIndex      = 240,
                SendFlg       =
                {
                    ClaimStatus = 1,
                    General     = 1,
                    Position    = 1,
                },
                Speed         = 40, -- FIXME: Should be 100 while chasing
                SpeedBase     = 40,
                Hpp           = 100,
                server_status = 1,
                Flags1        =
                {
                    CliPosInitFlag = 1,
                    Gender         = 1,
                    GraphSize      = 1,
                    LinkDeadFlag   = 1,
                    LinkShellFlag  = 1,
                    MonsterFlag    = 1,
                },
                Flags2        =
                {
                    g = 12,
                },
                Flags3        =
                {
                    MonStat = 6,
                    unknown_2_3 = 1,
                },
            },
            { -- Ascending
                UniqueNo = 16912624,
                ActIndex = 240,
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
                    Gender = 1,
                    GraphSize = 1,
                    LinkDeadFlag = 1,
                    LinkShellFlag = 1,
                    MonsterFlag = 1,
                    TargetOffFlag = 1,
                },
                Flags2 =
                {
                    g = 12,
                },
                Flags3 =
                {
                    MonStat = 7,
                    unknown_2_3 = 1,
                    unknown_3_3 = 1,
                    unknown_3_4 = 1,
                },
            },
        },
    },
}
