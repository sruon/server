-----------------------------------
-- Area: Hazhalm Testing Grounds
--   Mob: Vanquished Einherjar (DRK) (Einherjar)
-------------------------------------
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    -- Full immunity to dark sleep
    mob:addImmunity(xi.immunity.DARK_SLEEP)

    -- Casts instantly then every 30 seconds
    mob:setMobMod(xi.mobMod.MAGIC_DELAY, 0)
    mob:setMobMod(xi.mobMod.MAGIC_COOL, 30)
end

return entity
