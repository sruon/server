-----------------------------------
-- Area: Hazhalm Testing Grounds
--   NM: Ariri Samariri (Einherjar)
-----------------------------------
mixins =
{
    require('scripts/mixins/draw_in'),
}
-----------------------------------
-- Standard Poroggo TP moves + Frog Chorus
-- Water Bomb randomly resets hate. Ariri will run back to its spawn at increased speed when all reset.
-- Has increased regain below 25% (+200?)
-- Unverified/unimplemented claims:
--  - Resistance to damage increases as HP decreases.
--  - Black Mage spells appear to do half damage.
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    xi.einherjar.onBossInitialize(mob)
    mob:addImmunity(xi.immunity.BIND)
    mob:addImmunity(xi.immunity.GRAVITY)
    mob:addImmunity(xi.immunity.SILENCE)

    mob:setMobMod(xi.mobMod.MAGIC_COOL, 30)
end

entity.onMobSpawn = function(mob)
    mob:addListener('MAGIC_START', 'ARIRI_MAGIC_START', function(mobArg, spell, action)
        if mob:getLocalVar('providence') == 1 then
            mob:setSpellList(539)
            mob:setLocalVar('providence', 0)
            mob:setMobMod(xi.mobMod.MAGIC_COOL, 30)
        end
    end)
end

entity.onMobEngage = function(mob)
    mob:delStatusEffect(xi.effect.FLEE)
    mob:setMagicCastingEnabled(true)
end

entity.onMobFight = function(mob)
    if mob:getHPP() <= 25 then
        mob:setMod(xi.mod.REGAIN, 200)
    end
end

entity.onMobWeaponSkill = function(target, mob, skill)
    local skillId = skill:getID()

    if
        skillId == 1959 and -- water_bomb
        math.random(1, 100) <= 25
    then
        local enmityList = mob:getEnmityList()
        for _, enmity in ipairs(enmityList) do
            mob:clearEnmityForEntity(enmity.entity)
        end

        mob:disengage()
        mob:addStatusEffectEx(xi.effect.FLEE, 0, 10000, 0, 30)
        mob:setMagicCastingEnabled(false)
        mob:setMobMod(xi.mobMod.NO_AGGRO, 1)
        mob:timer(20000, function(mobArg)
            mobArg:setMobMod(xi.mobMod.NO_AGGRO, 0)
            mob:setMagicCastingEnabled(true)
        end)
    end
end

return entity
