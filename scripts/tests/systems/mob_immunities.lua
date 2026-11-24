describe('Mob immunities', function()
    local ID = zones[xi.zone.BONEYARD_GULLY]
    ---@type CClientEntityPair
    local player

    setup(function()
        player = xi.test.world:spawnPlayer({ zone = xi.zone.BONEYARD_GULLY })
    end)

    it('applies Parata immune_status', function()
        local parata = player.entities:get(ID.mob.PARATA)
        for _, immunity in ipairs({ xi.immunity.ADDLE, xi.immunity.GRAVITY, xi.immunity.BIND, xi.immunity.SILENCE }) do
            assert(parata:hasImmunity(immunity), string.format('Parata missing immunity %d', immunity))
        end
    end)

    it('applies Tuchulcha immune_status', function()
        local tuchulcha = player.entities:get(ID.mob.TUCHULCHA)
        for _, immunity in ipairs({
            xi.immunity.ADDLE,
            xi.immunity.GRAVITY,
            xi.immunity.BIND,
            xi.immunity.STUN,
            xi.immunity.PARALYZE,
            xi.immunity.LIGHT_SLEEP,
            xi.immunity.DARK_SLEEP,
        }) do
            assert(tuchulcha:hasImmunity(immunity), string.format('Tuchulcha missing immunity %d', immunity))
        end
    end)
end)
