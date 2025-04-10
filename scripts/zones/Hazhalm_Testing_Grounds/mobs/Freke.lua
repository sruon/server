-----------------------------------
-- Area: Hazhalm Testing Grounds
--   NM: Freke (Einherjar)
-----------------------------------
mixins =
{
    require('scripts/mixins/draw_in'),
}
-----------------------------------
-- Regular Cerberus moves
-- Immune to Silence for some reason
-- Below 25%: Uses 5x Lava Spit before GoH
---@type TMobEntity
local entity = {}

local function notBusy(mob)
    local action = mob:getCurrentAction()
    if
        action == xi.act.MOBABILITY_START or
        action == xi.act.MOBABILITY_USING or
        action == xi.act.MOBABILITY_FINISH
    then
        return false
    end

    return true
end

entity.onMobInitialize = function(mob)
    xi.einherjar.onBossInitialize(mob)
    mob:addImmunity(xi.immunity.SILENCE)
end

entity.onMobSpawn = function(mob)
    mob:setLocalVar('gohSequence', 0)
end

entity.onMobWeaponSkillPrepare = function(mob, target)
    local gohSequence = mob:getLocalVar('gohSequence')

    if gohSequence == 1 then
        return 1790 -- gates_of_hades
    elseif gohSequence ~= 0 then
        return 1785 -- lava_spit
    end

    if mob:getHPP() < 25 then
        -- 16.67% (1/6 possible TP moves) chance to start a GoH sequence
        if math.random(1, 10000) <= 1667 then
            mob:setLocalVar('gohSequence', 6)
            return 1785 -- lava_spit
        end
    end
end

entity.onMobWeaponSkill = function(target, mob, skill)
    local gohSequence = mob:getLocalVar('gohSequence')
    if gohSequence ~= 0 then
        if gohSequence > 1 and skill:getID() == 1785 then -- lava_spit
            mob:setLocalVar('gohSequence', gohSequence - 1)
            mob:setAutoAttackEnabled(false)
        elseif skill:getID() == 1790 then -- gates_of_hades
            mob:setLocalVar('gohSequence', 0)
            mob:setAutoAttackEnabled(true)
        end
    end
end

entity.onMobFight = function(mob, target)
    if notBusy(mob) and mob:getLocalVar('gohSequence') ~= 0 then
        mob:setTP(3000)
    end
end

return entity
