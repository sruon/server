-----------------------------------
-- Nosferatu's Kiss
-- Drains HP (Foe Level *0.5~1), MP, and TP.
-- Type: Magical
-- Utsusemi/Blink absorb: ignore shadow
-----------------------------------
---@type TMobSkill
local mobskillObject = {}

mobskillObject.onMobSkillCheck = function(target, mob, skill)
    return 0
end

mobskillObject.onMobWeaponSkill = function(target, mob, skill)
    -- Capture shows the following effects on a level 99 player from a level 85 mob:
    -- 108 HP drained
    -- 60 TP drained
    -- 25 MP drained
    local damage = math.random(mob:getMainLvl() / 2, mob:getMainLvl())

    damage = xi.mobskills.mobMagicalMove(mob, target, skill, damage, xi.element.DARK, 1, xi.mobskills.magicalTpBonus.MAB_BONUS, 1)
    damage = xi.mobskills.mobFinalAdjustments(damage, mob, skill, target, xi.attackType.MAGICAL, xi.damageType.DARK, xi.mobskills.shadowBehavior.IGNORE_SHADOWS)
    xi.mobskills.mobPhysicalDrainMove(mob, target, skill, xi.mobskills.drainType.HP, damage)
    -- TODO: Need captures for proper range!
    xi.mobskills.mobPhysicalDrainMove(mob, target, skill, xi.mobskills.drainType.MP, damage / 3)
    xi.mobskills.mobPhysicalDrainMove(mob, target, skill, xi.mobskills.drainType.TP, damage / 2)
    skill:setMsg(xi.msg.basic.SKILL_DRAIN_HP)

    return damage
end

return mobskillObject
