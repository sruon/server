-----------------------------------
-- Area: Hazhalm Testing Grounds
--   Mob: Djigga (Einherjar)
-------------------------------------
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    -- Attacks absorb buffs
    mob:setMobMod(xi.mobMod.ADD_EFFECT, 1)
end

entity.onAdditionalEffect = function(mob, target, damage)
    -- TODO: Implement absorb buff effect
    -- return xi.mob.onAddEffect(mob, target, damage, xi.mob.ae.STUN)
    return 0, 0, 0
end

return entity
