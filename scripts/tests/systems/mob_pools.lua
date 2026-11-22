describe('Mob pools', function()
    local lehkoHabhokaPool = 5922
    local alexanderPool    = 7087

    it('builds a trust from its pool', function()
        local player = xi.test.world:spawnPlayer({ zone = xi.zone.WEST_RONFAURE })

        player:spawnTrust(xi.magic.spell.LEHKO_HABHOKA)
        xi.test.world:skipTime(1)

        local trust
        for _, member in ipairs(player:getPartyWithTrusts()) do
            if member:isTrust() then
                trust = member
            end
        end

        assert(trust, 'trust was not summoned')
        assert(trust:getPool() == lehkoHabhokaPool, string.format('pool %u', trust:getPool()))
        assert(trust:getPacketName() == 'LehkoHabhoka', trust:getPacketName())
        assert(trust:getMainJob() == xi.job.THF, string.format('main job %u', trust:getMainJob()))
        assert(trust:getModelId() == 3026, string.format('model %u', trust:getModelId()))
        assert(trust:getMod(xi.mod.MPP) == 150, string.format('MPP %d', trust:getMod(xi.mod.MPP)))
    end)

    it('builds a pet from its pool', function()
        local player = xi.test.world:spawnPlayer({ zone = xi.zone.WEST_RONFAURE, job = xi.job.SMN, level = 75 })

        player:spawnPet(xi.petId.CARBUNCLE)

        local pet = player:getPet()
        assert(pet, 'pet was not summoned')
        assert(pet:getMainJob() == xi.job.WHM, string.format('main job %u', pet:getMainJob()))
        assert(pet:getModelId() == 16, string.format('model %u', pet:getModelId()))
        assert(pet:getMod(xi.mod.STUN_RES_RANK) == 0, string.format('stun rank %d', pet:getMod(xi.mod.STUN_RES_RANK)))
    end)

    it('builds an instance mob from its pool', function()
        local player = xi.test.world:spawnPlayer({ zone = xi.zone.ALZADAAL_UNDERSEA_RUINS })

        player:createInstance(7702)
        xi.test.world:tick(xi.tick.TIME)

        local alexander
        for _, mob in pairs(player:getInstance():getMobs()) do
            if mob:getName() == 'Alexander_WTC' then
                alexander = mob
            end
        end

        assert(alexander, 'Alexander_WTC not spawned')
        assert(alexander:getPool() == alexanderPool, string.format('pool %u', alexander:getPool()))
        assert(alexander:getMainJob() == xi.job.PLD, string.format('main job %u', alexander:getMainJob()))
        assert(alexander:getModelId() == 1834, string.format('model %u', alexander:getModelId()))
        assert(alexander:getMod(xi.mod.STUN_RES_RANK) == 5, string.format('stun rank %d', alexander:getMod(xi.mod.STUN_RES_RANK)))
    end)
end)
