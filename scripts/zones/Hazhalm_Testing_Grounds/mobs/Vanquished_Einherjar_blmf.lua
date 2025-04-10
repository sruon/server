-----------------------------------
-- Area: Hazhalm Testing Grounds
--   Mob: Vanquished Einherjar (BLM) (Einherjar)
-------------------------------------
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    -- Full immunity to dark sleep
    mob:addImmunity(xi.immunity.DARK_SLEEP)
end

return entity
