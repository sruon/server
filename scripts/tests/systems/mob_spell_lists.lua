describe('Mob spell lists', function()
    local player

    before_each(function()
        player = xi.test.world:spawnPlayer({ zone = xi.zone.CASTLE_OZTROJA })
    end)

    it('takes the spell list its template names', function()
        local mob = player.entities:get('Yagudo_Abbot')
        mob:respawn()

        assert(mob:getSpellListId() == xi.mobSpellList.BEASTMEN_WHM, string.format('expected the beastmen_whm list, got %d', mob:getSpellListId()))
        assert(mob:hasSpellList(), 'expected the abbot to have spells to cast')
    end)

    it('only offers spells inside the level range', function()
        local mob = player.entities:get('Yagudo_High_Priest')
        mob:respawn()
        assert(mob:getMainLvl() < 90, string.format('setup: expected a priest below level 90, got %d', mob:getMainLvl()))

        mob:setSpellList(xi.mobSpellList.YILBEGAN)
        assert(mob:getSpellListId() == xi.mobSpellList.YILBEGAN, string.format('expected the yilbegan list, got %d', mob:getSpellListId()))
        assert(not mob:hasSpellList(), 'expected no spells below level 90')

        mob:setSpellList(xi.mobSpellList.BEASTMEN_WHM)
        assert(mob:hasSpellList(), 'expected the beastmen_whm spells back')
    end)
end)
