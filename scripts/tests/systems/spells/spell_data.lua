describe('Spell data', function()
    ---@type CClientEntityPair
    local player

    before_each(function()
        player = xi.test.world:spawnPlayer(
            {
                job   = xi.job.BLU,
                level = 99,
            })
    end)

    it('loads spell fields from data/spells.yaml', function()
        local venomShell = GetSpell(xi.magic.spell.VENOM_SHELL)
        assert(venomShell:getSpellGroup() == xi.magic.spellGroup.BLUE, 'Venom Shell is not blue magic')
        assert(venomShell:getMPCost() == 86, string.format('Expected 86 MP, got %d', venomShell:getMPCost()))
        assert(venomShell:getCastTime() == 3000, string.format('Expected 3000 ms, got %d', venomShell:getCastTime()))
        assert(venomShell:getElement() == xi.element.WATER, 'Venom Shell is not water')
        assert(venomShell:getAoE() == xi.magic.aoe.RADIAL, 'Venom Shell is not radial')

        local cureIi = GetSpell(xi.magic.spell.CURE_II)
        assert(cureIi:getSpellFamily() == xi.magic.spellFamily.CURE, 'Cure II is not in the cure family')
        assert(cureIi:getCastTime() == 2250, string.format('Expected 2250 ms, got %d', cureIi:getCastTime()))
    end)

    it('applies blue spell mods and trait weight when set', function()
        local mnd    = player:getMod(xi.mod.MND)
        local mpHeal = player:getMod(xi.mod.MPHEAL)

        player:addSpell(xi.magic.spell.VENOM_SHELL)
        player:addSpell(xi.magic.spell.MAELSTROM)
        player.actions:setBlueSpells({ xi.magic.spell.VENOM_SHELL, xi.magic.spell.MAELSTROM })

        assert(player:getMod(xi.mod.MND) == mnd + 4, string.format('Expected MND +4, got %+d', player:getMod(xi.mod.MND) - mnd))
        assert(player:hasTrait(xi.trait.CLEAR_MIND), 'Two clear mind spells did not grant Clear Mind')
        assert(player:getMod(xi.mod.MPHEAL) == mpHeal + 3, string.format('Expected MPHEAL +3, got %+d', player:getMod(xi.mod.MPHEAL) - mpHeal))
    end)

    it('does not grant a trait below its points', function()
        player:addSpell(xi.magic.spell.VENOM_SHELL)
        player.actions:setBlueSpells({ xi.magic.spell.VENOM_SHELL })

        assert(not player:hasTrait(xi.trait.CLEAR_MIND), 'One clear mind spell granted Clear Mind')
    end)
end)
