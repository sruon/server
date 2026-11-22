describe('Mob skill data', function()
    ---@type CClientEntityPair
    local player

    ---@type CTestEntity
    local mob

    before_each(function()
        player = xi.test.world:spawnPlayer({ level = 99, zone = xi.zone.UPPER_DELKFUTTS_TOWER })
        mob    = player.entities:moveTo('Incubus_Bats')
        mob:respawn()
    end)

    it('reads distances and their default from data/mob_skills.yaml', function()
        assert(mob:getAbilityDistance(xi.mobSkill.KICK_OUT) == 10, 'kick out reaches 10 yalms')
        assert(mob:getAbilityDistance(xi.mobSkill.COMBO_1) == 7, 'combo uses the default distance')
    end)

    it('resolves the skills named by a skill list', function()
        mob:setMobMod(xi.mobMod.SKILL_LIST, 2)
        assert(mob:hasTPMoves(), 'the adamantoise list has skills')

        mob:setMobMod(xi.mobMod.SKILL_LIST, 1007)
        assert(not mob:hasTPMoves(), 'the ankabut list only names an unimplemented skill')
    end)

    it('uses a skill from data/mob_skills.yaml', function()
        mob:addTP(3000)
        mob:useMobAbility(xi.mobSkill.SONIC_BOOM_1, player, 0)
        xi.test.world:skipTime(4)
        xi.test.world:tickEntity(mob)
        assert(mob:getTP() < 3000, 'sonic boom spent the TP')
    end)
end)
