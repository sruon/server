-----------------------------------
-- Area: Caedarva Mire
--   NM: Khimaira
-----------------------------------
---@type TMobEntity
local entity = {}

entity.onMobSpawn = function(mob)
    mob:setMobMod(xi.mobMod.NO_MOVE, 0)
end

entity.onMobRoam = function(mob)
    mob:setMobMod(xi.mobMod.NO_MOVE, 0)
end

entity.onMobFight = function(mob, target)
    local targetPos = target:getPos()
    local drawInPositions =
    {
        { 839.093, -0.325, 366.537, targetPos.rot },
        { 842.545, -0.028, 364.163, targetPos.rot },
        { 849.699, -1.202, 361.752, targetPos.rot },
        { 851.793, -2.023, 369.038, targetPos.rot },
        { 845.684, -2.677, 376.167, targetPos.rot },
        { 839.907, -3.567, 380.276, targetPos.rot },
    }
    local drawInTable =
    {
        conditions =
        {
            target:getZPos() > 400,
            target:getZPos() < 350,
        },
        position = utils.randomEntry(drawInPositions),
        wait = 3,
    }
    for _, condition in ipairs(drawInTable.conditions) do
        if condition then
            mob:setMobMod(xi.mobMod.NO_MOVE, 1)
            utils.drawIn(target, drawInTable)
            break
        else
            mob:setMobMod(xi.mobMod.NO_MOVE, 0)
        end
    end
end

entity.onMobDeath = function(mob, player, optParams)
    player:addTitle(xi.title.KHIMAIRA_CARVER)
end

entity.onMobDespawn = function(mob)
    mob:setRespawnTime(math.random(48, 72) * 3600) -- 48 to 72 hours, in 1-hour increments
end

return entity
