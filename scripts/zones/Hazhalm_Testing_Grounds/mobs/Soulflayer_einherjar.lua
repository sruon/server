-----------------------------------
-- Area: Hazhalm Testing Grounds
--   Mob: Soulflayer (Einherjar)
-------------------------------------
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    -- Full immunity to blind and dark sleep
    mob:addImmunity(xi.immunity.BLIND)
    mob:addImmunity(xi.immunity.DARK_SLEEP)

    mob:setMobMod(xi.mobMod.MAGIC_COOL, 30)
end

return entity
