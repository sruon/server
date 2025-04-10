-----------------------------------
-- Area: Hazhalm Testing Grounds
--   NM: Hraesvelg (Einherjar)
-----------------------------------
mixins =
{
    require('scripts/mixins/job_special'),
    require('scripts/mixins/draw_in'),
}
-----------------------------------
-- Regular Manticore TP moves
-- Uses Mighty Strikes
-- Switches target randomly
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    xi.einherjar.onBossInitialize(mob)
    mob:addImmunity(xi.immunity.BIND)
    mob:addImmunity(xi.immunity.GRAVITY)
end

entity.onMobFight = function(mob)
    if os.time() >= mob:getLocalVar('resetEnmity') then
        local enmityList = mob:getEnmityList()

        for _, enmity in ipairs(enmityList) do
            mob:resetEnmity(enmity.entity)
        end

        local randomTarget = utils.randomEntry(enmityList)
        mob:addEnmity(randomTarget.entity, 30000, 30000)
        mob:updateTarget()

        mob:setLocalVar('resetEnmity', os.time() + math.random(5, 20))
    end
end

return entity
