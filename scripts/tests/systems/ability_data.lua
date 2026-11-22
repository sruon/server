describe('Ability data', function()
    it('loads targets, recasts, enmity and areas from data/abilities.yaml', function()
        local provoke = GetAbility(xi.jobAbility.PROVOKE)
        assert(provoke:getRecast() == 30, string.format('expected Provoke recast 30, got %d', provoke:getRecast()))
        assert(provoke:getRange() == 16, string.format('expected Provoke range 16, got %s', provoke:getRange()))
        assert(provoke:getCE() == 1 and provoke:getVE() == 1800, 'expected Provoke enmity 1/1800')
        assert(provoke:getAOE() == xi.aoeType.NONE, 'Provoke should be single target')

        local warcry = GetAbility(xi.jobAbility.WARCRY)
        assert(warcry:getAOE() == xi.aoeType.ROUND, 'Warcry should be an area ability')
        assert(warcry:getRadius() == 14, string.format('expected Warcry radius 14, got %d', warcry:getRadius()))

        local dustCloud = GetAbility(xi.jobAbility.DUST_CLOUD)
        assert(dustCloud:getAOE() == xi.aoeType.NONE, 'Dust Cloud should not be an area ability')
        assert(dustCloud:getRadius() == 10, string.format('expected Dust Cloud radius 10, got %d', dustCloud:getRadius()))

        assert(GetAbility(xi.jobAbility.SIC):getAddType() == xi.addType.CHARM_PET, 'Sic should need a charmed pet')
        assert(GetAbility(xi.jobAbility.ENRAGE) == nil, 'enum-only ids should not load an ability')
    end)

    describe('Berserk', function()
        it('cannot be used before level 15', function()
            local player = xi.test.world:spawnPlayer({ job = xi.job.WAR, level = 14 })

            player.actions:useAbility(player, xi.jobAbility.BERSERK)
            xi.test.world:tick()

            player.assert.no:hasEffect(xi.effect.BERSERK)
        end)

        it('is learned at level 15 and puts its recast up', function()
            local player = xi.test.world:spawnPlayer({ job = xi.job.WAR, level = 15 })

            player.actions:useAbility(player, xi.jobAbility.BERSERK)
            xi.test.world:tick()

            player.assert:hasEffect(xi.effect.BERSERK)
            assert(player:hasRecast(xi.recast.ABILITY, GetAbility(xi.jobAbility.BERSERK):getRecastID()), 'Berserk recast should be running')
        end)
    end)
end)
