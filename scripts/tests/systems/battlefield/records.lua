-----------------------------------
-- Battlefield records
-- The fastest clear outlives the battlefield and is offered to the next registration.
-----------------------------------
local entryEventId = 32000
local entryNpc     = 'SD_Entrance'

local function spawnCandidate()
    local player = xi.test.world:spawnPlayer({ zone = xi.zone.MONARCH_LINN })
    player:addMission(xi.mission.log_id.COP, xi.mission.id.cop.ANCIENT_VOWS)
    player:setCharVar('Mission[6][248]Status', 2)

    return player
end

-- First free arena, asked one arena at a time like the client does
local function enterAncientVows(player)
    local content = xi.battlefield.contents[xi.battlefield.id.ANCIENT_VOWS]
    local replies = {}

    player.entities:gotoAndTrigger(entryNpc)
    for arena = 1, 3 do
        local reply = player.events:updateWithPosition(entryEventId, bit.lshift(content.index, 4) + arena, { x = 0, y = 0, z = 0 }) or 0
        table.insert(replies, reply)

        if reply == xi.battlefield.returnCode.CUTSCENE then
            break
        end
    end

    player.events:finish(entryEventId, 0)

    local battlefield = player:getBattlefield()
    assert(battlefield, string.format('no arena was granted, replies %s', table.concat(replies, ', ')))

    return battlefield
end

local function settleBattlefields()
    for _ = 1, 3 do
        xi.test.world:skipTime(11)
        xi.test.world:tick()
    end
end

describe('Battlefield records', function()
    local players

    before_each(function()
        players = {}
    end)

    after_each(function()
        for _, player in ipairs(players) do
            if player:getBattlefield() then
                player:leaveBattlefield(xi.battlefield.leaveCode.EXIT)
            end
        end

        settleBattlefields()
    end)

    it('never offers a record slower than the time limit', function()
        local player = spawnCandidate()
        table.insert(players, player)

        local battlefield             = enterAncientVows(player)
        local _, clearTime, partySize = battlefield:getRecord()
        assert(clearTime <= battlefield:getTimeLimit(), string.format('record of %d seconds is over the time limit', clearTime))
        assert(partySize <= 6, 'a record party is at most one party')
    end)

    it('keeps a win for the next registration', function()
        local winner = spawnCandidate()
        table.insert(players, winner)

        local won             = enterAncientVows(winner)
        local timeLimit       = won:getTimeLimit()
        local _, standingTime = won:getRecord()
        winner.bcnm:killMobs()
        winner.bcnm:expectWin({ finishOption = 2 })
        settleBattlefields()

        local challenger = spawnCandidate()
        table.insert(players, challenger)

        local name, recordTime = enterAncientVows(challenger):getRecord()
        assert(recordTime < timeLimit, string.format('no record was kept, %s holds %d seconds', name, recordTime))
        assert(recordTime <= standingTime, string.format('record of %d seconds replaced a faster %d', recordTime, standingTime))
        assert(name ~= 'nobody', 'the record should name its holder')
    end)
end)
