-----------------------------------
-- Area: Hazhalm Testing Grounds
--   NM: Barri (Einherjar; Motsognir add)
-----------------------------------
local ID = zones[xi.zone.HAZHALM_TESTING_GROUNDS]
-------------------------------------
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    mob:addImmunity(xi.immunity.PETRIFY)
    mob:addImmunity(xi.immunity.TERROR)
    mob:setMobMod(xi.mobMod.SUPERLINK, ID.mob.MOTSOGNIR)
end

return entity
