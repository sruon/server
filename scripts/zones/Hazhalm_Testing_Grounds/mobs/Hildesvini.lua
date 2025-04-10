-----------------------------------
-- Area: Hazhalm Testing Grounds
--   NM: Hildesvini (Einherjar)
-----------------------------------
mixins =
{
    require('scripts/mixins/draw_in'),
}
local DJIGGAS = zones[xi.zone.HAZHALM_TESTING_GROUNDS].mob.DJIGGA_HILDESVINI
-----------------------------------
-- Regular Marid abilities
-- Spawns 3 Djigga after each TP move
-- Prefers Proboscis Shower if Djiggas are alive
---@type TMobEntity
local entity = {}

local function despawnDjiggas()
    for _, mobId in ipairs(DJIGGAS) do
        local djigga = GetMobByID(mobId)
        if djigga and djigga:isSpawned() then
            DespawnMob(mobId)
        end
    end
end

entity.onMobInitialize = function(mob)
    xi.einherjar.onBossInitialize(mob)
end

entity.onMobSpawn = function(mob)
    despawnDjiggas()
end

entity.onMobWeaponSkillPrepare = function(mob, target)
    local anyAlive = utils.any(DJIGGAS, function(_, mobId)
        local djigga = GetMobByID(mobId)
        if djigga and djigga:isAlive() then
            return true
        end
    end)

    if anyAlive then
        return 1708 -- Proboscis Shower
    end
end

entity.onMobWeaponSkill = function(target, mob, skill)
    local chamberId = mob:getLocalVar('[ein]chamber')
    local chamberData = xi.einherjar.getChamber(chamberId)

    local freeIds = {}

    for _, mobId in ipairs(DJIGGAS) do
        local djigga = GetMobByID(mobId)
        if djigga and not djigga:isSpawned() then
            table.insert(freeIds, mobId)
        end
    end

    for i = 1, math.min(3, #freeIds) do
        local djigga = GetMobByID(freeIds[i])
        if djigga and not djigga:isSpawned() then
            djigga:setSpawn(
                mob:getXPos() + i,
                mob:getYPos(),
                mob:getZPos(),
                mob:getRotPos()
            )
            if chamberData then
                xi.einherjar.spawnMob(djigga, 3, chamberData)
            else -- fallback for testing with no einherjar context
                djigga:spawn()
            end
        end
    end
end

entity.onMobDespawn = function(mob)
    despawnDjiggas()
end

return entity
