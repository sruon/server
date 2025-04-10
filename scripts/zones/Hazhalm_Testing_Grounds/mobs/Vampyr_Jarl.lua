-----------------------------------
-- Area: Hazhalm Testing Grounds
--   NM: Vampyr Jarl (Einherjar)
-----------------------------------
mixins =
{
    require('scripts/mixins/draw_in'),
}
local VAMPYR_WOLF = zones[xi.zone.HAZHALM_TESTING_GROUNDS].mob.VAMPYR_WOLF
local VAMPYR_BATS = zones[xi.zone.HAZHALM_TESTING_GROUNDS].mob.VAMPYR_BATS
-----------------------------------
-- Vampyr TP moves
-- Immune to Bind/Gravity/Blind/Silence
-- Casts every 30 seconds
-- About every 2 minutes, upon using Nocturnal Servitude, will split into 6x Bats or 6x Wolves
-- The mobs will run around for a minute, before pathing to a common location and "merging" back into Jarl
---@type TMobEntity
local entity = {}

local function despawnAllAdds()
    local allAdds = {}

    for _, id in ipairs(VAMPYR_BATS) do
        table.insert(allAdds, id)
    end
    for _, id in ipairs(VAMPYR_WOLF) do
        table.insert(allAdds, id)
    end

    for _, mobId in ipairs(allAdds) do
        local add = GetMobByID(mobId)
        if add and add:isSpawned() then
            DespawnMob(mobId)
        end
    end
end

local function vanish(mob)
    mob:setMobMod(xi.mobMod.NO_MOVE, 1)
    mob:setMobMod(xi.mobMod.NO_AGGRO, 1)
    if mob:isEngaged() then
        mob:setMobMod(xi.mobMod.NO_REST, 1)
    end
    mob:setAutoAttackEnabled(false)
    mob:setMagicCastingEnabled(false)
    mob:setMobAbilityEnabled(false)
    mob:setStatus(xi.status.INVISIBLE)
    mob:hideName(true)
    mob:setUntargetable(true)
end

local function reset(mob)
    mob:setMobMod(xi.mobMod.NO_MOVE, 0)
    mob:setMobMod(xi.mobMod.NO_AGGRO, 0)
    mob:setMobMod(xi.mobMod.NO_REST, 0)
    mob:hideName(false)
    mob:setUntargetable(false)
    mob:setStatus(xi.status.UPDATE)
    mob:setAutoAttackEnabled(true)
    mob:setMagicCastingEnabled(true)
    mob:setMobAbilityEnabled(true)
    mob:setLocalVar('nextTransform', os.time() + 120)
end

local function disableInteractions(mob)
    mob:disengage()
    mob:setMobMod(xi.mobMod.NO_AGGRO, 1)
    mob:setMod(xi.mod.UDMGPHYS, -10000)
    mob:setMod(xi.mod.UDMGMAGIC, -10000)
    mob:setMod(xi.mod.UDMGRANGE, -10000)
    mob:setMod(xi.mod.UDMGBREATH, -10000)
end

local function allAddsGrouped(mob, jarl)
    local mobIds = jarl:getLocalVar('addsFamily') == 1 and VAMPYR_BATS or VAMPYR_WOLF

    for _, mobId in ipairs(mobIds) do
        if mobId ~= mob:getID() then
            local add = GetMobByID(mobId)
            if add and add:isAlive() then
                if mob:checkDistance(add) > 5 then
                    return false
                end
            end
        end
    end

    return true
end

local function onAddRoam(mob, jarl)
    local despawnTime   = mob:getLocalVar('despawnAt')
    local isPathingHome = mob:getLocalVar('isPathingHome')

    -- One of the 6 adds is invincible and will not engage
    if mob:getLocalVar('invincible') ~= 0 then
        disableInteractions(mob)
    end

    if despawnTime >= os.time() and not mob:isFollowingPath() then
        local pos = mob:getPos()
        mob:pathTo(pos.x + math.random(-30, 30), pos.y, pos.z + math.random(-30, 30), 9) -- Pathflags = 9 (xi.pathflag.run, xi.pathflag.scripted)
        return
    end

    if despawnTime <= os.time() then
        local spawnPos = mob:getSpawnPos()
        -- Spawn position is where Jarl is
        mob:pathTo(spawnPos.x, spawnPos.y, spawnPos.z, 9)
        if isPathingHome == 0 then
            disableInteractions(mob)
            mob:setLocalVar('isPathingHome', 1)
        else
            -- If all bats are grouped, despawn. This will make Jarl reappear.
            if allAddsGrouped(mob, jarl) then
                DespawnMob(mob:getID())
            elseif mob:checkDistance(jarl) < 5 then
                mob:setMobMod(xi.mobMod.NO_MOVE, 1)
                mob:setMobMod(xi.mobMod.ROAM_DISTANCE, 0)
                mob:setMobMod(xi.mobMod.ROAM_COOL, 0)
                mob:setMobMod(xi.mobMod.DONT_ROAM_HOME, 0)
            end
        end
    end
end

local function onAddDeath(mob)
    if mob:isAlive() then
        -- Unclear if it's possible for Jarl to die while hidden
        local newHpp = math.max(mob:getHPP() - 6, 1)
        mob:setHP(mob:getMaxHP() * newHpp / 100)
    end
end

local function onAddFight(mob)
    if mob:getLocalVar('invincible') ~= 0 then
        mob:disengage()
    end

    if mob:getLocalVar('despawnAt') <= os.time() then
        disableInteractions(mob)
    end
end

local function checkReappear(mob)
    if mob:getStatus() == xi.status.INVISIBLE then
        local allAddsDespawned = true
        local addsMobs = mob:getLocalVar('addsFamily') == 1 and VAMPYR_BATS or VAMPYR_WOLF

        for _, mobId in ipairs(addsMobs) do
            local add = GetMobByID(mobId)
            if add and add:isAlive() then
                allAddsDespawned = false
            end
        end

        if allAddsDespawned then
            reset(mob)
        end
    end
end

entity.onMobInitialize = function(mob)
    xi.einherjar.onBossInitialize(mob)
    mob:addImmunity(xi.immunity.BIND)
    mob:addImmunity(xi.immunity.BLIND)
    mob:addImmunity(xi.immunity.GRAVITY)
    mob:addImmunity(xi.immunity.SILENCE)

    mob:setMobMod(xi.mobMod.MAGIC_DELAY, 0)
    mob:setMobMod(xi.mobMod.MAGIC_COOL, 30)
end

entity.onMobDespawn = function(mob)
    -- should not be necessary but just in case
    despawnAllAdds()
end

entity.onMobSpawn = function(mob)
    despawnAllAdds()
end

entity.onMobEngage = function(mob)
    reset(mob)
end

entity.onMobFight = function(mob, target)
    -- Check if all adds have despawned
    checkReappear(mob)
end

entity.onMobRoam = function(mob)
    checkReappear(mob)
end

entity.onMobWeaponSkill = function(target, mob, skill)
    if
        skill:getID() == 2112 and -- nocturnal_servitude
        mob:getLocalVar('nextTransform') <= os.time()
    then
        local selectedMobs
        vanish(mob)
        if math.random(1, 2) == 1 then
            selectedMobs = VAMPYR_BATS
            mob:setLocalVar('addsFamily', 1)
        else
            selectedMobs = VAMPYR_WOLF
            mob:setLocalVar('addsFamily', 2)
        end

        local randomInvincibleMob = utils.randomEntry(selectedMobs)
        for _, mobId in ipairs(selectedMobs) do
            local add = GetMobByID(mobId)
            if add then
                add:setSpawn(mob:getXPos(), mob:getYPos(), mob:getZPos(), mob:getRotPos())
                add:addListener('DEATH', 'JARL_DEATH', function(_)
                    onAddDeath(mob)
                end)
                add:addListener('ROAM_TICK', 'JARL_ROAM', function(_)
                    onAddRoam(add, mob)
                end)
                add:addListener('COMBAT_TICK', 'JARL_COMBAT_TICK', function(_)
                    onAddFight(add)
                end)
                add:spawn()
                add:setLocalVar('despawnAt', os.time() + 120)
                if mobId == randomInvincibleMob then
                    add:setLocalVar('invincible', 1)
                end
            end
        end
    end
end

return entity
