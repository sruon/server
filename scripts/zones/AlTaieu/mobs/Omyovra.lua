-----------------------------------
-- Area: Al'Taieu
--  Mob: Om'yovra
-----------------------------------
mixins =
{
    require('scripts/mixins/special_roam')({
        idleAnim    = 7,
        fightAnim   = 6,
        returnDelay = 15,
    }),
}
-----------------------------------
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    mob:setMobMod(xi.mobMod.ADD_EFFECT, 1)
    mob:setMobMod(xi.mobMod.NO_STANDBACK, 1)
end

entity.onMobSpawn = function(mob)
    mob:setMod(xi.mod.REGEN, 50)
    mob:setMobMod(xi.mobMod.WEAPON_BONUS, mob:getMainLvl() - 2) -- Base damage is level * 2
    mob:setMod(xi.mod.AGI, 76 - mob:getStat(xi.mod.AGI))        -- Jimmy indicates AGI for Omyovra should be 76 or so
    -- Yovra have a +50% bonus to defense
    mob:addMod(xi.mod.DEF, mob:getStat(xi.mod.DEF) * 0.5)
end

entity.onAdditionalEffect = function(mob, target, damage)
    return xi.mob.onAddEffect(mob, target, damage, xi.mob.ae.PARALYZE, { duration = math.random(5, 10) })
end

entity.onMobDeath = function(mob, player, optParams)
end

return entity
