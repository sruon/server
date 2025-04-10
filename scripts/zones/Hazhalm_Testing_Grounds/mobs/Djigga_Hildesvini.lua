-----------------------------------
-- Area: Hazhalm Testing Grounds
--   Mob: Djigga (Einherjar; Hildesvini Add)
-------------------------------------
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    -- Immune to Bind, Sleep, Gravity
    mob:addImmunity(xi.immunity.BIND)
    mob:addImmunity(xi.immunity.DARK_SLEEP)
    mob:addImmunity(xi.immunity.LIGHT_SLEEP)
    mob:addImmunity(xi.immunity.GRAVITY)

    -- Attacks absorb buffs
    mob:setMobMod(xi.mobMod.ADD_EFFECT, 1)
end

entity.onAdditionalEffect = function(mob, target, damage)
    -- TODO: Implement absorb buff effect
    -- return xi.mob.onAddEffect(mob, target, damage, xi.mob.ae.STUN)
    return 0, 0, 0
end

return entity
