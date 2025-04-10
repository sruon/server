-----------------------------------
-- Area: Hazhalm Testing Grounds
--   NM: Mokkuralfi (Einherjar)
-----------------------------------
mixins =
{
    require('scripts/mixins/draw_in'),
}
-----------------------------------
-- Regular Flan TP moves (except Synergysm)
-- Casts various tier 2/3 -ga spells and enfeebs with low cooldown
-- At low HP, uses Xenoglossia once.
-- With Xenoglossia active, casts Thundaga IV instantly. Does not use it without it.
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    xi.einherjar.onBossInitialize(mob)
    mob:addImmunity(xi.immunity.SILENCE)

    mob:setMobMod(xi.mobMod.MAGIC_COOL, 10)
end

entity.onMobWeaponSkillPrepare = function(mob, target)
    if
        mob:getHPP() <= 20 and
        mob:getLocalVar("Xenoglossia") == 0
    then
        mob:setLocalVar("Xenoglossia", 1)
        return 1823 -- xenoglossia
    end
end

entity.onMobMagicPrepare = function(mob, target, spellId)
    if mob:getMod(xi.mod.UFASTCAST) >= 100 then
        return 197 -- thundaga_iv
    end
end

return entity
