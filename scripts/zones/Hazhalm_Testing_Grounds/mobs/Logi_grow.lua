-----------------------------------
-- Area: Hazhalm Testing Grounds
--   Mob: Logi (Growing) (Einherjar)
-------------------------------------
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    -- Immune to all immobilizing effects
    mob:addImmunity(xi.immunity.PETRIFY)
    mob:addImmunity(xi.immunity.DARK_SLEEP)
    mob:addImmunity(xi.immunity.LIGHT_SLEEP)
    mob:addImmunity(xi.immunity.STUN)
    mob:addImmunity(xi.immunity.TERROR)

    -- Starts casting immediately then every 30 seconds
    mob:setMobMod(xi.mobMod.MAGIC_DELAY, 0)
    mob:setMobMod(xi.mobMod.MAGIC_COOL, 30)
end

entity.onMobFight = function(mob)
    -- TODO: Implement growth mechanic + self destruct
end

return entity
