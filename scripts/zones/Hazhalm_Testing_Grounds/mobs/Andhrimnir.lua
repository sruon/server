-----------------------------------
-- Area: Hazhalm Testing Grounds
--   NM: Andhrimnir (Einherjar)
-----------------------------------
mixins =
{
    require('scripts/mixins/draw_in'),
}
-----------------------------------
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
end

entity.onMobSpawn = function(mob)
end

entity.onMobDeath = function(mob, player, optParams)
end

return entity
