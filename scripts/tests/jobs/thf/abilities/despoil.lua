describe('Despoil', function()
    ---@type CClientEntityPair
    local player

    setup(function()
        player = xi.test.world:spawnPlayer({ zone = xi.zone.BOSTAUNIEUX_OUBLIETTE })
    end)

    it('maps a despoiled item to its listed effect', function()
        assert(player:getDespoilDebuff(xi.item.CHUNK_OF_MYTHRIL_ORE) == xi.effect.DEFENSE_DOWN, 'expected Mythril Ore to land Defense Down')
        assert(player:getDespoilDebuff(xi.item.SLICE_OF_LAND_CRAB_MEAT) == xi.effect.SLOW, 'expected Land Crab Meat to land Slow')
    end)

    it('returns 0 for an item with no listed effect', function()
        assert(player:getDespoilDebuff(xi.item.MOAT_CARP_1) == 0, 'expected no effect for an unlisted item')
    end)
end)
