-----------------------------------
-- Area: Hazhalm Testing Grounds
--   Mob: Waldgeist (Einherjar)
-------------------------------------
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    -- 30 seconds between casts
    mob:setMobMod(xi.mobMod.MAGIC_COOL, 30)
end

return entity
