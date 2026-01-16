-----------------------------------
-- Amphiptere family mixin
-- Handles Reaving Wind aura animation logic only
-- Use with special_roam mixin for dive/surface behavior
-----------------------------------

local function amphiptere(mob)
    mob:addListener('WEAPONSKILL_USE', 'REAVING_WIND_AURA', function(mobArg, target, actionId, tp, action)
        -- Amphipteres gain a temporary aura following the use of reaving wind.
        if actionId == xi.mobSkill.REAVING_WIND then
            mobArg:setAnimationSub(2)
            -- Zirnitra spams a knockback while aura is active
            mobArg:setLocalVar('auraEndTime', GetSystemTime() + 20)
        end
    end)

    mob:addListener('WEAPONSKILL_STATE_EXIT', 'SPAM_KNOCKBACK', function(mobArg, actionId)
        if actionId == xi.mobSkill.REAVING_WIND then
            mobArg:useMobAbility(xi.mobSkill.REAVING_WIND_KNOCKBACK)
        elseif actionId == xi.mobSkill.REAVING_WIND_KNOCKBACK then
            if GetSystemTime() >= mobArg:getLocalVar('auraEndTime') then
                mobArg:setLocalVar('auraEndTime', 0)
                mobArg:setAnimationSub(5) -- Back to fight anim
            else
                mobArg:useMobAbility(xi.mobSkill.REAVING_WIND_KNOCKBACK)
            end
        end
    end)
end

return amphiptere
