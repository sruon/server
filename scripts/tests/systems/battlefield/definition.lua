-----------------------------------
-- Battlefield definitions
-- Limits and rules come from data/zones/<zone>/battlefields.yaml, not the battlefield script.
-----------------------------------
local function settleBattlefields()
    for _ = 1, 3 do
        xi.test.world:skipTime(11)
        xi.test.world:tick()
    end
end

describe('Battlefield definitions', function()
    local player

    after_each(function()
        if player and player:getBattlefield() then
            player:leaveBattlefield(xi.battlefield.leaveCode.EXIT)
        end

        settleBattlefields()
    end)

    it('falls back to a 30 minute fight for one party', function()
        player = xi.test.world:spawnPlayer({ zone = xi.zone.MONARCH_LINN })
        player:addMission(xi.mission.log_id.COP, xi.mission.id.cop.ANCIENT_VOWS)
        player:setCharVar('Mission[6][248]Status', 2)
        player.bcnm:enter('SD_Entrance', xi.battlefield.id.ANCIENT_VOWS)

        local battlefield = player:getBattlefield()
        assert(battlefield:getTimeLimit() == utils.minutes(30), string.format('time limit is %d seconds', battlefield:getTimeLimit()))
        assert(battlefield:getMaxParticipants() == 6, string.format('max participants is %d', battlefield:getMaxParticipants()))
    end)

    it('caps players to the level the zone file sets', function()
        player = xi.test.world:spawnPlayer({ zone = xi.zone.HORLAIS_PEAK, job = xi.job.WAR, level = 75 })
        player:addItem(xi.item.STAR_ORB)
        player.bcnm:enter('BC_Entrance', xi.battlefield.id.TAILS_OF_WOE, { xi.item.STAR_ORB })

        local restriction = player:getStatusEffect(xi.effect.LEVEL_RESTRICTION)
        assert(restriction, 'no level restriction was applied')
        assert(restriction:getPower() == 40 + xi.settings.map.BATTLE_CAP_TWEAK, string.format('capped to %d', restriction:getPower()))
    end)

    it('hands the framework the rules it reads', function()
        local definition = GetBattlefieldDefinition(xi.battlefield.id.AMAN_TROVE_MARS_HORLAIS_PEAK)

        assert(definition, 'no definition for A.M.A.N. Trove')
        assert(definition.delayToExit == 4, string.format('exit delay is %d', definition.delayToExit))
        assert(definition.allowTrusts, 'trusts should be allowed')
        assert(definition.hasWipeGrace, 'wipe grace should default to on')
        assert(not definition.cleanupDebuffs, 'debuff cleanup should default to off')
        assert(GetBattlefieldDefinition(xi.battlefield.id.RANK_2_MISSION_1 + 31) == nil, 'an unlisted id has no definition')
    end)
end)
