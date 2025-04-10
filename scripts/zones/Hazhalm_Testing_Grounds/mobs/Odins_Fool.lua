-----------------------------------
-- Area: Hazhalm Testing Grounds
--   Mob: Odin's Fool (Einherjar)
-------------------------------------
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    -- 20 seconds between casts
    mob:setMobMod(xi.mobMod.MAGIC_COOL, 20)
end

return entity
