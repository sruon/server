-----------------------------------
-- Area: Hazhalm Testing Grounds
--   Mob: Dark Elemental (Einherjar)
-------------------------------------
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    -- Elemental resistance to Dark (Frazzle, Dispel)
    -- Full immunity to blind, dark sleep
    mob:addImmunity(xi.immunity.BLIND)
    mob:addImmunity(xi.immunity.DARK_SLEEP)

    -- Aggros by sound and magic, link by sound
    mob:setMobMod(xi.mobMod.DETECTION, bit.bor(xi.detects.MAGIC, xi.detects.HEARING))

    -- Starts casting immediately then every 30 seconds
    mob:setMobMod(xi.mobMod.MAGIC_DELAY, 0)
    mob:setMobMod(xi.mobMod.MAGIC_COOL, 30)
end

return entity
