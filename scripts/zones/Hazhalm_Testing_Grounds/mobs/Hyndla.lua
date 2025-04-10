-----------------------------------
-- Area: Hazhalm Testing Grounds
--   Mob: Hyndla (Einherjar)
-------------------------------------
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    -- Full immunity to dark sleep, silence
    mob:addImmunity(xi.immunity.DARK_SLEEP)
    mob:addImmunity(xi.immunity.SILENCE)

    -- Has no known TP moves or spells
end

return entity
