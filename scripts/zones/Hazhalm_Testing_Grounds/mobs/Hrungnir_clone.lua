-----------------------------------
-- Area: Hazhalm Testing Grounds
--   NM: Hrungnir (Clone) (Einherjar)
-----------------------------------
mixins =
{
    require('scripts/mixins/draw_in'),
}
local ID = zones[xi.zone.HAZHALM_TESTING_GROUNDS]
-----------------------------------
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    xi.einherjar.onBossInitialize(mob)
end

entity.onMobSpawn = function(mob)
end

entity.onMobDeath = function(mob, player, optParams)
end

return entity
