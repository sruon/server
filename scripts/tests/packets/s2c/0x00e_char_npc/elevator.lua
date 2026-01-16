-- Elevator (SubKind 3) packet tests
-- Tests for elevators with associated gates
-- Retail captures from NPCPacketLog/Nours/npc_packets_Metalworks.lua
--
-- South Elevator IDs:
--   17748038 = South Elevator Platform (@6l1)
--   17748036 = South Upper Elevator Door (_6ls)
--   17748037 = South Lower Elevator Door (_6lt)
--
-- Retail sequence (elevator started DOWN, went UP, then DOWN):
--   [22:55:35] Elevator UP (10) + Lower Door closes (9)
--   [22:55:43] Upper Door opens (8)
--   [22:55:54] Elevator DOWN (11) + Upper Door closes (9)
--   [22:56:02] Lower Door opens (8)

---@type CharNpcTestSuite
return
{
    ['Metalworks South Elevator'] =
    {
        test = function(player)
            player:gotoZone(xi.zone.METALWORKS)
            -- Position near South elevator
            player.actions:move(-55.978, -12.020, -13.100)
            xi.test.world:tick()

            -- Get elevator entity
            local elevator = player.entities:get(17748038)

            -- Elevator starts at BOTTOM with lower door open
            -- Sequence: startElevator -> ASCEND + lower door closes
            --           arriveElevator -> TOP + upper door opens
            --           startElevator -> DESCEND + upper door closes
            --           arriveElevator -> BOTTOM + lower door opens

            elevator:startElevator() -- Elevator UP (10) + Lower Door closes (9)
            xi.test.world:tick()

            elevator:arriveElevator() -- Upper Door opens (8)
            xi.test.world:tick()

            elevator:startElevator() -- Elevator DOWN (11) + Upper Door closes (9)
            xi.test.world:tick()

            elevator:arriveElevator() -- Lower Door opens (8)
            xi.test.world:tick()
        end,
        expected =
        {
            -- [22:55:35] Elevator going UP (server_status = 10)
            {
                UniqueNo      = 17748038,
                SendFlg       =
                {
                    ClaimStatus = 1,
                    General     = 1,
                    Position    = 1,
                },
                Flags0        =
                {
                    KingFlag = 1,
                    MovTime  = 1,
                },
                Speed         = 50,
                SpeedBase     = 50,
                Hpp           = 100,
                server_status = 10,
                Flags1        =
                {
                    CliPosInitFlag = 1,
                    GraphSize      = 1,
                    TargetOffFlag  = 1,
                },
                SubKind       = 3,
                Data          =
                {
                    DoorId  = 829175360, -- "@6l1"
                    EndTime = 8,
                },
            },
            -- [22:55:35] Lower Door closes (server_status = 9)
            {
                UniqueNo      = 17748037,
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
                Speed         = 50,
                SpeedBase     = 50,
                Hpp           = 100,
                server_status = 9,
                Flags1        =
                {
                    CliPosInitFlag = 1,
                    GraphSize      = 1,
                    TalkUcoffFlag  = 1,
                    TargetOffFlag  = 1,
                },
                SubKind       = 2,
                Data          =
                {
                    DoorId = 1953248863, -- "_6lt"
                },
            },
            -- [22:55:43] Upper Door opens (server_status = 8)
            {
                UniqueNo      = 17748036,
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
                Speed         = 50,
                SpeedBase     = 50,
                Hpp           = 100,
                server_status = 8,
                Flags1        =
                {
                    CliPosInitFlag = 1,
                    GraphSize      = 1,
                    TalkUcoffFlag  = 1,
                    TargetOffFlag  = 1,
                },
                SubKind       = 2,
                Data          =
                {
                    DoorId = 1936471647, -- "_6ls"
                },
            },
            -- [22:55:54] Elevator going DOWN (server_status = 11)
            {
                UniqueNo      = 17748038,
                SendFlg       =
                {
                    ClaimStatus = 1,
                    General     = 1,
                    Position    = 1,
                },
                Flags0        =
                {
                    KingFlag = 1,
                    MovTime  = 1,
                },
                Speed         = 50,
                SpeedBase     = 50,
                Hpp           = 100,
                server_status = 11,
                Flags1        =
                {
                    CliPosInitFlag = 1,
                    GraphSize      = 1,
                    TargetOffFlag  = 1,
                },
                SubKind       = 3,
                Data          =
                {
                    DoorId  = 829175360, -- "@6l1"
                    EndTime = 8,
                },
            },
            -- [22:55:54] Upper Door closes (server_status = 9)
            {
                UniqueNo      = 17748036,
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
                Speed         = 50,
                SpeedBase     = 50,
                Hpp           = 100,
                server_status = 9,
                Flags1        =
                {
                    CliPosInitFlag = 1,
                    GraphSize      = 1,
                    TalkUcoffFlag  = 1,
                    TargetOffFlag  = 1,
                },
                SubKind       = 2,
                Data          =
                {
                    DoorId = 1936471647, -- "_6ls"
                },
            },
            -- [22:56:02] Lower Door opens (server_status = 8)
            {
                UniqueNo      = 17748037,
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
                Speed         = 50,
                SpeedBase     = 50,
                Hpp           = 100,
                server_status = 8,
                Flags1        =
                {
                    CliPosInitFlag = 1,
                    GraphSize      = 1,
                    TalkUcoffFlag  = 1,
                    TargetOffFlag  = 1,
                },
                SubKind       = 2,
                Data          =
                {
                    DoorId = 1953248863, -- "_6lt"
                },
            },
        },
    },
}
