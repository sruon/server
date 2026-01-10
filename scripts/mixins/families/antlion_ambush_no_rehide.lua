-----------------------------------
-- Antlion family mixin (For antlions that don't return underground)
-----------------------------------
require('scripts/globals/mixins')
-----------------------------------
g_mixins = g_mixins or {}
g_mixins.families = g_mixins.families or {}

g_mixins.families.antlion_ambush_no_rehide = function(antlion)
    antlion:addListener('PRESPAWN', 'ANTLION_AMBUSH_PRESPAWN', function(mob)
        -- Retail: HideFlag=0 on spawn (no ??? shown until player has seen mob)
        mob:setUntargetable(true)
        mob:setAutoAttackEnabled(false)
        mob:setAnimationSub(4)  -- Retail: MonStat=4 for underground
        mob:setMobMod(xi.mobMod.NO_MOVE, 1)
    end)

    antlion:addListener('ENGAGE', 'ANTLION_AMBUSH_ENGAGE', function(mob, target)
        if mob:getLocalVar('[Ambush]Done') == 0 then
            mob:useMobAbility(xi.mobSkill.PIT_AMBUSH_1)
        end
    end)

    -- Ensures an interrupted pit ambush doesn't let the mob stay hidden underground
    antlion:addListener('WEAPONSKILL_STATE_EXIT', 'ANTLION_AMBUSH_FINISH', function(mob, skillId)
        if skillId == xi.mobSkill.PIT_AMBUSH_1 then
            mob:setHideFlag(false)  -- Clear ??? display
            mob:setUntargetable(false)
            mob:setAutoAttackEnabled(true)
            mob:setAnimationSub(5, false)  -- Retail: MonStat=5 for above ground
            mob:setMobMod(xi.mobMod.NO_MOVE, 0)
            mob:setLocalVar('[Ambush]Done', 1)
        end
    end)
end

return g_mixins.families.antlion_ambush_no_rehide
