describe('Automaton status removal', function()
    ---@type CClientEntityPair
    local player

    ---@type CTestEntity
    local mob

    before_each(function()
        player = xi.test.world:spawnPlayer(
            {
                job   = xi.job.PUP,
                level = 75,
                zone  = xi.zone.CEIZAK_BATTLEGROUNDS,
            })

        mob = player.entities:moveTo('Blanched_Mandragora')
        mob:respawn()
        player:setUnkillable(true)

        player:unlockAttachment(xi.item.HARLEQUIN_FRAME)
        player:unlockAttachment(xi.item.SOULSOOTHER_HEAD)
        player:setAutomatonFrame(xi.automaton.frame.HARLEQUIN)
        player:setAutomatonHead(xi.automaton.head.SOULSOOTHER)
        player:setSkillLevel(xi.skill.AUTOMATON_MAGIC, 2000)
    end)

    local function engageWith(effect)
        player:addStatusEffect(effect, { power = 1, duration = 300, tick = 3, origin = player })
        player:spawnPet(xi.petId.AUTOMATON)

        local pet = player:getPet()
        assert(pet)

        pet:engage(mob:getTargID())

        for _ = 1, 6 do
            xi.test.world:tickEntity(pet)
            xi.test.world:skipTime(3)
        end
    end

    it('casts poisona on a poisoned master', function()
        engageWith(xi.effect.POISON)

        assert(not player:hasStatusEffect(xi.effect.POISON), 'expected the soulsoother to cure poison')
    end)

    it('casts silena on a silenced master', function()
        engageWith(xi.effect.SILENCE)

        assert(not player:hasStatusEffect(xi.effect.SILENCE), 'expected the soulsoother to cure silence')
    end)
end)
