-----------------------------------
-- Area: Hazhalm Testing Grounds
--   Mob: Rotting Huskarl (BLM) (Einherjar)
-------------------------------------
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    -- Full immunity to dark sleep
    mob:addImmunity(xi.immunity.DARK_SLEEP)

    -- No delay, then every 25 seconds. No standback
    mob:setMobMod(xi.mobMod.NO_STANDBACK, 1)
    mob:setMobMod(xi.mobMod.MAGIC_DELAY, 0)
    mob:setMobMod(xi.mobMod.MAGIC_COOL, 25)
end

return entity
