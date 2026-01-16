-----------------------------------
-- Hpemde family mixin
-- Handles combat-specific behavior (mouth states, auto-attack enable on damage)
-- Use with special_roam mixin for dive/surface behavior
-----------------------------------

-- Hpemde take 100% increased damage and deal 2x base damage in open mouth form
local function openMouth(mob)
    mob:setMobMod(xi.mobMod.WEAPON_BONUS, mob:getMainLvl() + 2)
    mob:setMod(xi.mod.DMG, 10000)
    mob:setAnimationSub(3)
    mob:wait(2000)
end

local function closeMouth(mob)
    mob:setMobMod(xi.mobMod.WEAPON_BONUS, 0)
    mob:setMod(xi.mod.DMG, 0)
    mob:setLocalVar('[hpemde]changeTime', mob:getBattleTime() + 30)
    mob:setAnimationSub(6)
    mob:wait(2000)
end

local function hpemde(mob)
    mob:addListener('SPAWN', 'HPEMDE_SPAWN', function(m)
        m:setMod(xi.mod.REGEN, 10)
        m:setAutoAttackEnabled(false)
        m:setMobAbilityEnabled(false)
    end)

    mob:addListener('ROAM_TICK', 'HPEMDE_RTICK', function(m)
        if m:getHPP() == 100 then
            m:setLocalVar('[hpemde]damaged', 0)
        end
    end)

    mob:addListener('ENGAGE', 'HPEMDE_ENGAGE', function(m, target)
        m:setLocalVar('[hpemde]disengageTime', m:getBattleTime() + 45)
    end)

    mob:addListener('MAGIC_TAKE', 'HPEMDE_MAGIC_TAKE', function(target, caster, spell)
        target:setLocalVar('[hpemde]disengageTime', target:getBattleTime() + 45)
    end)

    mob:addListener('COMBAT_TICK', 'HPEMDE_CTICK', function(m)
        if m:getLocalVar('[hpemde]damaged') == 0 then
            local disengageTime = m:getLocalVar('[hpemde]disengageTime')

            if m:getHP() < m:getMaxHP() then
                m:setAutoAttackEnabled(true)
                m:setMobAbilityEnabled(true)
                m:setLocalVar('[hpemde]damaged', 1)
                m:setLocalVar('[hpemde]changeTime', m:getBattleTime() + 30)
            elseif disengageTime > 0 and m:getBattleTime() > disengageTime then
                m:setLocalVar('[hpemde]disengageTime', 0)
                m:disengage()
            end
        else
            if
                m:getAnimationSub() == 6 and
                m:getBattleTime() > m:getLocalVar('[hpemde]changeTime')
            then
                openMouth(m)
            elseif
                m:getAnimationSub() == 3 and
                m:getHP() < m:getLocalVar('[hpemde]closeMouthHP')
            then
                closeMouth(m)
            end
        end
    end)

    mob:addListener('TAKE_DAMAGE', 'HPEMDE_TAKE_DAMAGE', function(m, damage, attacker, attackType, damageType)
        if
            m:getAnimationSub() == 3 and
            damage >= 250
        then
            closeMouth(m)
        end
    end)
end

return hpemde
