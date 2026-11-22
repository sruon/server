describe('Instances', function()
    it('Waking the Colossus spawns Alexander', function()
        local player = xi.test.world:spawnPlayer({ zone = xi.zone.ALZADAAL_UNDERSEA_RUINS })

        player:createInstance(7702)
        xi.test.world:tick(xi.tick.TIME)

        local instance = player:getInstance()
        assert(instance and instance:getID() == 7702)

        for _, mob in pairs(instance:getMobs()) do
            if mob:getName() == 'Alexander_WTC' then
                return
            end
        end

        assert(false, 'Alexander_WTC not spawned')
    end)

    it('respawns an instanced mob after its timer expires', function()
        local player = xi.test.world:spawnPlayer({ zone = xi.zone.ALZADAAL_UNDERSEA_RUINS })

        player:createInstance(7702)
        xi.test.world:tick(xi.tick.TIME)

        local alexander
        for _, mob in pairs(player:getInstance():getMobs()) do
            if mob:getName() == 'Alexander_WTC' then
                alexander = player.entities:get(mob)
                break
            end
        end

        assert(alexander, 'Alexander_WTC not spawned')

        alexander:setRespawnTime(300)
        alexander:despawn()
        alexander.assert.no:isSpawned()

        xi.test.world:skipTime(305)
        xi.test.world:tick(xi.tick.SPAWN)

        alexander.assert:isSpawned()
    end)

    it('reads its settings from the zone instances file', function()
        local player = xi.test.world:spawnPlayer({ zone = xi.zone.ALZADAAL_UNDERSEA_RUINS })

        player:createInstance(7702)
        xi.test.world:tick(xi.tick.TIME)

        local instance = player:getInstance()
        assert(instance:getName() == 'waking_the_colossus')
        assert(instance:getEntranceZoneID() == xi.zone.ALZADAAL_UNDERSEA_RUINS)
        assert(instance:getTimeLimit() == 15 * 60)
    end)

    it('loads only the entities tagged with its name', function()
        local function entityNames(player, id)
            player:createInstance(id)
            xi.test.world:tick(xi.tick.TIME)

            local instance = player:getInstance()
            assert(instance and instance:getID() == id)

            local names = {}
            for _, mob in pairs(instance:getMobs()) do
                names[mob:getName()] = true
            end

            for _, npc in pairs(instance:getNpcs()) do
                names[npc:getName()] = true
            end

            return names
        end

        local goldenSalvage = entityNames(xi.test.world:spawnPlayer({ zone = xi.zone.ARRAPAGO_REEF }), 5500)
        local extermination = entityNames(xi.test.world:spawnPlayer({ zone = xi.zone.ARRAPAGO_REEF }), 5502)

        assert(goldenSalvage['Percipient_Fish'], 'Golden Salvage should have its fish')
        assert(not extermination['Percipient_Fish'], 'Extermination should not have Golden Salvage fish')
        assert(extermination['Carrion_Crab'], 'Extermination should have its crabs')
        assert(not goldenSalvage['Carrion_Crab'], 'Golden Salvage should not have Extermination crabs')
        assert(goldenSalvage['Ancient_Lockbox'] and extermination['Ancient_Lockbox'], 'the shared lockbox should load in both')
    end)
end)
