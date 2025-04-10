-----------------------------------
-- Area: Hazhalm Testing Grounds
--   NM: Vampyr Wolf (Einherjar; Vampyr Jarl adds)
-----------------------------------
-- When Vampyr Jarl uses Nocturnal Servitude, 6 copies of this mob spawn.
-- On spawn, they run in random directions, similar to Terror'd Promyvion bosses. They do not aggro.
-- All are immune to Bind, Gravity, Sleeps, Petrify, Stun and Terror
-- One of the 6 is also immune to every source of damage or enfeebs.
-- It will also not engage the players, even if a direct action is performed.
-- Every mob killed removes ~6% HP from Jarl
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    mob:addImmunity(xi.immunity.BIND)
    mob:addImmunity(xi.immunity.GRAVITY)
    mob:addImmunity(xi.immunity.DARK_SLEEP)
    mob:addImmunity(xi.immunity.LIGHT_SLEEP)
    mob:addImmunity(xi.immunity.PETRIFY)
    mob:addImmunity(xi.immunity.TERROR)
    mob:addImmunity(xi.immunity.STUN)
end

entity.onMobSpawn = function(mob)
    mob:setMobMod(xi.mobMod.DONT_ROAM_HOME, 1)
    mob:setMobMod(xi.mobMod.ROAM_COOL, 8)
    mob:setMobMod(xi.mobMod.ROAM_DISTANCE, 60)
    mob:setMobMod(xi.mobMod.ROAM_RATE, 5)
    mob:setMobMod(xi.mobMod.NO_AGGRO, 1)
    mob:setMobMod(xi.mobMod.NO_LINK, 1)
end

return entity
