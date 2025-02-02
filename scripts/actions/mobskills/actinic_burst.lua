-----------------------------------
-- Actinic Burst
-- Family: Ghrah
-- Description: Greatly lowers the accuracy of enemies within range for a brief period of time.
-- Type: Magical (Light)
-- Utsusemi/Blink absorb: Ignores shadows
-- Range: Unknown
-----------------------------------
---@type TMobSkill
local mobskillObject = {}

mobskillObject.onMobSkillCheck = function(target, mob, skill)
    return 0
end

mobskillObject.onMobWeaponSkill = function(target, mob, skill)
    local power = 400 * (1 + (math.random(-10, 10) / 100)) -- +/- 10%
    local duration = 16

    skill:setMsg(xi.mobskills.mobStatusEffectMove(mob, target, xi.effect.FLASH, power, 1, duration))

    return xi.effect.FLASH
end

return mobskillObject
