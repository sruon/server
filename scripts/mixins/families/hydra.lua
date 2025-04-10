require('scripts/globals/mixins')

g_mixins = g_mixins or {}
g_mixins.families = g_mixins.families or {}

g_mixins.families.hydra = function(hydraMob)
    -- 15% chance to destroy one head (its right, then its left)
    -- Head grows back based on time
    -- 0 -> 1 = 3 to 2 heads
    -- 1 -> 2 = 2 to 1 heads
    -- 2 -> 1 = 1 to 2 heads, plays regrow anim
    -- 1 -> 0 = 2 to 3 heads, plays regrow anim
    hydraMob:addListener('CRITICAL_TAKE', 'HYDRA_CRITICAL_TAKE', function(mob)
        local battletime      = mob:getBattleTime()
        local broken          = mob:getAnimationSub()
        local headBreakChance = (mob:getLocalVar('headBreakChance') ~= 0 and mob:getLocalVar('headBreakChance')) or 15
        local headRegrowMin   = (mob:getLocalVar('headRegrowMin') ~= 0 and mob:getLocalVar('headRegrowMin')) or 60
        local headRegrowMax   = (mob:getLocalVar('headRegrowMin') ~= 0 and mob:getLocalVar('headRegrowMin')) or 90

        if math.random(1, 100) <= headBreakChance and broken < 2 then
            mob:setAnimationSub(broken + 1)
            mob:setLocalVar('headgrow', battletime + math.random(headRegrowMin, headRegrowMax))
        end
    end)

    hydraMob:addListener('COMBAT_TICK', 'HYDRA_COMBAT_TICK', function(mob)
        local battletime    = mob:getBattleTime()
        local headgrow      = mob:getLocalVar('headgrow')
        local broken        = mob:getAnimationSub()
        local headRegrowMin = (mob:getLocalVar('headRegrowMin') ~= 0 and mob:getLocalVar('headRegrowMin')) or 60
        local headRegrowMax = (mob:getLocalVar('headRegrowMin') ~= 0 and mob:getLocalVar('headRegrowMin')) or 90

        if headgrow < battletime and broken > 0 then
            mob:setAnimationSub(broken - 1)
            mob:setLocalVar('headgrow', battletime + math.random(headRegrowMin, headRegrowMax))
        end
    end)
end

return g_mixins.families.hydra
