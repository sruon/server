-----------------------------------
-- Area: Hazhalm Testing Grounds
--   Mob: Berserkr (BLM) (Einherjar)
-------------------------------------
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    -- Full immunity to petrify
    mob:addImmunity(xi.immunity.PETRIFY)
end

return entity
