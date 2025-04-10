-----------------------------------
-- Heliovoid
-- Absorbs one positive status effect from each target, including food.
-- Type: Magical
-- Utsusemi/Blink absorb: Ignores
-- Range: AoE
-----------------------------------
---@type TMobSkill
local mobskillObject = {}

mobskillObject.onMobSkillCheck = function(target, mob, skill)
    return 0
end

mobskillObject.onMobWeaponSkill = function(target, mob, skill)
    -- TODO: This does not currently take food into account
    local result = mob:stealStatusEffect(target)
    skill:setMsg(xi.msg.basic.EFFECT_DRAINED)

    if result ~= 0 then
        return 1
    end

    skill:setMsg(xi.msg.basic.SKILL_MISS)
    return 0
end

return mobskillObject
