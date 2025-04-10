-----------------------------------
-- Area: Hazhalm Testing Grounds
--   NM: Grimgerde (Einherjar; Odin add)
-----------------------------------
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    mob:addImmunity(xi.immunity.PETRIFY)
end

return entity
