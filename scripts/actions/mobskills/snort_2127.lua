-----------------------------------
-- Snort (Himinrjot auto-attacks)
-- Description: Deals Wind damage to targets in a fan-shaped area of effect. Additional effect: Knockback
-- Type: Magical (Wind)
-----------------------------------
---@type TMobSkill
local mobskillObject = {}

mobskillObject.onMobSkillCheck = function(target, mob, skill)
    return 0
end

mobskillObject.onMobWeaponSkill = function(target, mob, skill)
    -- TODO: Given the spammy nature, the damage might be lower
    local damage = mob:getWeaponDmg() * 4

    damage = xi.mobskills.mobMagicalMove(mob, target, skill, damage, xi.element.WIND, 1, xi.mobskills.magicalTpBonus.NO_EFFECT)
    damage = xi.mobskills.mobFinalAdjustments(damage, mob, skill, target, xi.attackType.MAGICAL, xi.damageType.WIND, xi.mobskills.shadowBehavior.IGNORE_SHADOWS)

    -- TODO: What about others in fan-shaped area of effect?
    target:takeDamage(damage, mob, xi.attackType.MAGICAL, xi.damageType.WIND)
    skill:setMsg(xi.msg.basic.HIT_DMG)

    -- TODO: Might be a complete enmity reset
    mob:lowerEnmity(target, 25)
    return damage
end

return mobskillObject
