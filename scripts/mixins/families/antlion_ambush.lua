-----------------------------------
-- Antlion family mixin (For antlions that return underground)
-----------------------------------
require('scripts/globals/mixins')
-----------------------------------
g_mixins = g_mixins or {}
g_mixins.families = g_mixins.families or {}

-- Hide underground without showing ??? (for initial spawn)
local function hideUnderground(mob)
    mob:hideName(true)
    mob:setUntargetable(true)
    mob:setCollisionDisabled(true)
    mob:setAutoAttackEnabled(false)
    mob:setAnimationSub(4)  -- Retail: MonStat=4 for underground
    mob:setMobMod(xi.mobMod.NO_MOVE, 1)
end

-- Hide underground with ??? display (for returning after combat)
local function hideWithFlag(mob)
    hideUnderground(mob)
    mob:setHideFlag(true)  -- Retail: HideFlag=1 only when returning underground
end

g_mixins.families.antlion_ambush = function(antlion)
    antlion:addListener('PRESPAWN', 'ANTLION_AMBUSH_PRESPAWN', function(mob)
        -- Retail: HideFlag=0 on spawn (no ??? shown until player has seen mob)
        hideUnderground(mob)
    end)

    antlion:addListener('ENGAGE', 'ANTLION_AMBUSH_ENGAGE', function(mob, target)
        -- mob:setStatus(xi.status.UPDATE)
        mob:useMobAbility(xi.mobSkill.PIT_AMBUSH_1)
    end)

    antlion:addListener('WEAPONSKILL_STATE_EXIT', 'ANTLION_AMBUSH_FINISH', function(mob, skillId)
        if skillId == xi.mobSkill.PIT_AMBUSH_1 then
            mob:hideName(false)
            mob:setHideFlag(false)  -- Clear ??? display if returning from disengage
            mob:setUntargetable(false)
            mob:setCollisionDisabled(false)
            mob:setAutoAttackEnabled(true)
            mob:setAnimationSub(5)  -- Retail: MonStat=5 for above ground
            mob:setMobMod(xi.mobMod.NO_MOVE, 0)
        end
    end)

    antlion:addListener('DISENGAGE', 'ANTLION_AMBUSH_DISENGAGE', function(mob)
        -- Retail: HideFlag=1 when returning underground after combat (shows ???)
        hideWithFlag(mob)
    end)
end

return g_mixins.families.antlion_ambush
