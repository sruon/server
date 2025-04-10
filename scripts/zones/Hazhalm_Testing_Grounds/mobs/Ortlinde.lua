-----------------------------------
-- Area: Hazhalm Testing Grounds
--   NM: Ortlinde (Einherjar; Odin add)
-----------------------------------
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    mob:addImmunity(xi.immunity.PETRIFY)
end

return entity
