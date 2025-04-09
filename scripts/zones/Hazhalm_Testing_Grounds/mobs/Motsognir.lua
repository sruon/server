-----------------------------------
-- Area: Hazhalm Testing Grounds
--   NM: Motsognir (Einherjar)
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
    mob:addMod(xi.mod.UDMGPHYS, -10000)
    mob:addMod(xi.mod.UDMGMAGIC, -10000)
    mob:addMod(xi.mod.UDMGRANGE, -10000)
    mob:addMod(xi.mod.UDMGBREATH, -10000)
    mob:setMobMod(xi.mobMod.NO_STANDBACK, 1)
    -- TODO: Superlinking is not working quite right.
    -- Motsognir ends up with enmity but does not engage
    -- See onMobRoamAction for current workaround
    mob:setMobMod(xi.mobMod.SUPERLINK, mob:getID())
end

entity.onMobFight = function(mob, target)
    if mob:getLocalVar('despawning') == 1 then
        return
    end

    -- Motsognir despawns when all 12 demons are dead
    -- Its HP is reduced by 1/12th of its max HP for each demon that dies
    local demonsAlive = 0
    for i = ID.mob.HERVARTH, ID.mob.HADDING_THE_YOUNGER do
        local demon = GetMobByID(i)
        if demon and demon:isAlive() then
            demonsAlive = demonsAlive + 1
        end
    end

    local maxHP = mob:getMaxHP()
    local expectedHP = maxHP * (demonsAlive / 12)
    if mob:getHP() > expectedHP then
        mob:setHP(math.max(expectedHP, 1))
    end

    -- Timer so Motsognir doesn't despawn before the demons nameplates are gone
    if demonsAlive == 0 then
        mob:setLocalVar('despawning', 1)
        mob:timer(10000, function()
            DespawnMob(mob:getID())
        end)
    end
end

entity.onMobSpawn = function(mob)
    mob:setLocalVar('despawning', 0)
end

entity.onMobRoamAction = function(mob)
    -- Temporary workaround to force Motsognir to engage on superlinking
    for i = ID.mob.HERVARTH, ID.mob.HADDING_THE_YOUNGER do
        local demon = GetMobByID(i)
        if demon and demon:isEngaged() then
            local target = demon:getTarget()
            if target then
                mob:updateEnmity(target)
                return
            end
        end
    end
end

entity.onMobDespawn = function(mob)
    mob:setLocalVar('despawning', 0)
end

return entity
