describe('Job point gifts', function()
    local function giftDelta(job, mod)
        local player = xi.test.world:spawnPlayer({ job = job, level = 99 })
        local before = player:getMod(mod)
        player:masterJob()

        return player:getMod(mod) - before
    end

    it('SAM Hasso and Seigan cap +10', function()
        local capDelta = giftDelta(xi.job.SAM, xi.mod.ZANSHIN_CAP)
        assert(capDelta == 10, string.format('cap +%d', capDelta))
    end)

    it('SAM Zanshin +10', function()
        local zanshinDelta = giftDelta(xi.job.SAM, xi.mod.ZANSHIN)
        assert(zanshinDelta == 10, string.format('zanshin +%d', zanshinDelta))
    end)

    it('RNG Barrage +1 shot', function()
        local barrageDelta = giftDelta(xi.job.RNG, xi.mod.BARRAGE_COUNT)
        assert(barrageDelta == 1, string.format('barrage +%d', barrageDelta))
    end)

    it('BLU magic defense +36', function()
        local defenseDelta = giftDelta(xi.job.BLU, xi.mod.MDEF)
        assert(defenseDelta == 36, string.format('magic defense +%d', defenseDelta))
    end)

    it('SMN Avatar\'s Favor +1', function()
        local favorDelta = giftDelta(xi.job.SMN, xi.mod.AVATARS_FAVOR_ENHANCE)
        assert(favorDelta == 1, string.format('avatar\'s favor +%d', favorDelta))
    end)
end)
