-----------------------------------
-- Area: Hazhalm Testing Grounds
--   Mob: Craven Einherjar (Bhoot) (Einherjar)
-------------------------------------
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    -- Full immunity to paralyze
    -- 12 yalms standback
    mob:addImmunity(xi.immunity.PARALYZE)
    mob:setMobMod(xi.mobMod.STANDBACK_RANGE, 12)
    mob:setMobMod(xi.mobMod.MAGIC_COOL, 30)
end

return entity
