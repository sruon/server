-----------------------------------
-- Obfuscate
-- Description: A blinding wave hits players in an area of effect.
-- Type: Enfeebling
-- Utsusemi/Blink absorb: Ignores shadows
-- Range: 10' radial
-----------------------------------
---@type TMobSkill
local mobskillObject = {}

mobskillObject.onMobSkillCheck = function(target, mob, skill)
    return 0
end

mobskillObject.onMobWeaponSkill = function(target, mob, skill)
    local power = 600 * (1 + (math.random(-10, 10) / 100)) -- +/- 10%
    local duration = 20

    skill:setMsg(xi.mobskills.mobStatusEffectMove(mob, target, xi.effect.FLASH, power, 1, duration))

    return xi.effect.FLASH
end

return mobskillObject
