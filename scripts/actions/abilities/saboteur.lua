-----------------------------------
-- Ability: Saboteur
-- If the next spell you cast is enfeebling magic, its effect and duration will be enhanced.
-- Obtained: Red Mage Level 83
-- Recast Time: 5:00
-- Duration: 1:00
-----------------------------------
---@type TAbility
local abilityObject = {}

abilityObject.onAbilityCheck = function(player, target, ability)
    return 0, 0
end

abilityObject.onUseAbility = function(player, target, ability)
    xi.job_utils.red_mage.useSaboteur(player, target, ability)
end

return abilityObject
