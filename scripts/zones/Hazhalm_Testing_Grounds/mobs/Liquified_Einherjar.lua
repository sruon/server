-----------------------------------
-- Area: Hazhalm Testing Grounds
--   Mob: Liquified Einherjar (Einherjar)
-------------------------------------
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    -- No delay, then every 25 seconds.
    mob:setMobMod(xi.mobMod.MAGIC_DELAY, 0)
    mob:setMobMod(xi.mobMod.MAGIC_COOL, 25)
end

return entity
