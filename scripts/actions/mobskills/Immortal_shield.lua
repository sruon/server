-----------------------------------
-- Immortal Shield
-- Description: Grants a Magic Shield effect for a time.
-- Type: Enhancing
-- Range: Self
-----------------------------------
---@type TMobSkill
local mobskillObject = {}

mobskillObject.onMobSkillCheck = function(target, mob, skill)
    return 0
end

mobskillObject.onMobWeaponSkill = function(target, mob, skill)
    -- TODO: The correct effect is:
    --  - Grants a Stoneskin-like effect that absorbs 100% of all magic damage up until unknown cap
    --  - It does not outright resist magic, enfeebs can still stick
    --  - The effect lasts until cap is reached
    mob:setAnimationSub(2) -- 2 orbiting purple shields
    mob:addStatusEffect(xi.effect.MAGIC_SHIELD, 0, 0, 0, 45)
    skill:setMsg(xi.msg.basic.SKILL_GAIN_EFFECT)
    return xi.effect.MAGIC_SHIELD
end

return mobskillObject
