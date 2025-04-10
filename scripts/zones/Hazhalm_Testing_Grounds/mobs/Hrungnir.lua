-----------------------------------
-- Area: Hazhalm Testing Grounds
--   NM: Hrungnir (Einherjar)
-----------------------------------
mixins =
{
    require('scripts/mixins/draw_in'),
}
local CLONE = zones[xi.zone.HAZHALM_TESTING_GROUNDS].mob.HRUNGNIR_CLONE
-----------------------------------
-- 2 copies are spawned, each must be defeated within 60 seconds of the other (counting from death time)
-- If not, the dead copy gets respawned with full HP.
-- Use regular Golem TP moves, along with an Ice Break that shows as a regular attack.
-- TODO: Ice Break shows "Special Attack"
---@type TMobEntity
local entity = {}

local function spawnClone(mob)
    local chamberId = mob:getLocalVar("[ein]chamber")
    local chamberData = xi.einherjar.getChamber(chamberId)
    local clone = GetMobByID(CLONE)
    if clone then
        clone:setSpawn(mob:getXPos() + 1, mob:getYPos(), mob:getZPos(), mob:getRotPos())
        if chamberData then
            xi.einherjar.spawnMob(clone, 2, chamberData)
        else -- fallback for testing with no einherjar context
            clone:spawn()
        end
    end
end

entity.onMobInitialize = function(mob)
    xi.einherjar.onBossInitialize(mob)
end

entity.onMobSpawn = function(mob)
    mob:setLocalVar('cloneToD', 0)
    -- Slight delay to wait for the local var to be setup
    -- TODO: To be removed once the localvars/mobmods being reset on spawn is sorted out
    mob:timer(200, function()
        spawnClone(mob)
    end)
end

entity.onMobFight = function(mob, target)
    -- Repop Clone every 60 seconds if Main is up and Clone is not.
    local clone = GetMobByID(CLONE)
    local cloneToD = mob:getLocalVar('cloneToD')

    if
        clone and
        clone:getCurrentAction() == xi.act.NONE and
        os.time() > cloneToD + 60
    then
        spawnClone(mob)
        clone:updateEnmity(target)
    end
end

entity.onMobDeath = function(mob, player, optParams)
    local clone = GetMobByID(CLONE)
    if clone and clone:isAlive() then
        clone:setLocalVar('mainToD', os.time())
    end
end

return entity
