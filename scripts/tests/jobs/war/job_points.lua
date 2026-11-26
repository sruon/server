describe('Job points', function()
    ---@type CClientEntityPair
    local player

    before_each(function()
        player = xi.test.world:spawnPlayer({ job = xi.job.WAR, level = 99 })
    end)

    it('no gifts at 0 job points', function()
        assert(player:getMod(xi.mod.SUPERIOR_LEVEL) == 0, 'superior level at 0 job points')
        assert(player:getJobPointLevel(xi.jp.MIGHTY_STRIKES_EFFECT) == 0, 'mighty strikes effect at 0 job points')
    end)

    it('mastered job gets all gifts and categories', function()
        player:masterJob()

        local superiorLevel = player:getMod(xi.mod.SUPERIOR_LEVEL)
        assert(superiorLevel == 5, string.format('superior %d', superiorLevel))

        local critRate = player:getMod(xi.mod.CRITHITRATE)
        assert(critRate >= 5, string.format('crit rate %d', critRate))

        assert(player:getJobPointLevel(xi.jp.MIGHTY_STRIKES_EFFECT) == 20, 'mighty strikes effect below 20')
        assert(player:getJobPointLevel(xi.jp.DOUBLE_ATTACK_EFFECT) == 20, 'double attack effect below 20')
    end)

    it('gifts drop on job change', function()
        player:masterJob()
        player:changeJob(xi.job.MNK)
        player:setLevel(99)

        assert(player:getMod(xi.mod.SUPERIOR_LEVEL) == 0, 'warrior gifts kept on monk')
    end)
end)
