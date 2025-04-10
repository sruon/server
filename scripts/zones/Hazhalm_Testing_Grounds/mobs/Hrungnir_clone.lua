-----------------------------------
-- Area: Hazhalm Testing Grounds
--   NM: Hrungnir (Clone) (Einherjar)
-----------------------------------
mixins =
{
    require('scripts/mixins/draw_in'),
}
local MAIN = zones[xi.zone.HAZHALM_TESTING_GROUNDS].mob.HRUNGNIR
-----------------------------------
-- 2 copies are spawned, each must be defeated within 60 seconds of the other (counting from death time)
-- If not, the dead copy gets respawned with full HP.
-- Use regular Golem TP moves, along with an Thunder Break that shows as a regular attack.
-- TODO: Thunder Break shows "Special Attack"
---@type TMobEntity
local entity = {}

local function spawnMain(mob)
    local chamberId = mob:getLocalVar("[ein]chamber")
    local chamberData = xi.einherjar.getChamber(chamberId)
    local clone = GetMobByID(MAIN)
    if chamberData and clone then
        clone:setSpawn(mob:getXPos() + 1, mob:getYPos(), mob:getZPos(), mob:getRotPos())
        xi.einherjar.spawnMob(clone, 2, chamberData)
    end
end

entity.onMobInitialize = function(mob)
    xi.einherjar.onBossInitialize(mob)
end

entity.onMobSpawn = function(mob)
    mob:setLocalVar('mainToD', 0)
end

entity.onMobFight = function(mob, target)
    -- Repop Main every 60 seconds if Clone is up and Main is not.
    local clone = GetMobByID(MAIN)
    local cloneToD = mob:getLocalVar('mainToD')

    if
        clone and
        clone:getCurrentAction() == xi.act.NONE and
        os.time() > cloneToD + 60
    then
        spawnMain(mob)
        clone:updateEnmity(target)
    end
end

entity.onMobDeath = function(mob, player, optParams)
    local clone = GetMobByID(MAIN)
    if clone and clone:isAlive() then
        clone:setLocalVar('cloneToD', os.time())
    end
end

return entity
