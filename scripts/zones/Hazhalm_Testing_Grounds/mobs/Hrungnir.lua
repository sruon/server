-----------------------------------
-- Area: Hazhalm Testing Grounds
--   NM: Hrungnir (Einherjar)
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
end

entity.onMobSpawn = function(mob)
    -- Slight delay to wait for the local var to be setup
    -- TODO: To be removed once the localvars/mobmods being reset on spawn is sorted out
    mob:timer(200, function()
        local chamberId = mob:getLocalVar("[ein]chamber")
        local chamberData = xi.einherjar.getChamber(chamberId)
        local clone = GetMobByID(ID.mob.HRUNGNIR_CLONE)
        if chamberData and clone then
            clone:setSpawn(
                mob:getXPos() + 1,
                mob:getYPos(),
                mob:getZPos(),
                mob:getRotPos()
            )
            xi.einherjar.spawnMob(clone, 2, chamberData)
        end
    end)
end

entity.onMobDeath = function(mob, player, optParams)
end

return entity
