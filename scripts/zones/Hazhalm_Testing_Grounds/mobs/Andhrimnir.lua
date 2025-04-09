-----------------------------------
-- Area: Hazhalm Testing Grounds
--   NM: Andhrimnir (Einherjar)
-----------------------------------
mixins =
{
    require('scripts/mixins/draw_in'),
}
-----------------------------------
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    xi.einherjar.onBossInitialize(mob)
    mob:addImmunity(xi.immunity.BIND)
    mob:addImmunity(xi.immunity.BLIND)
    mob:addImmunity(xi.immunity.GRAVITY)

    -- Silence is unconfirmed
    mob:addImmunity(xi.immunity.SILENCE)
end

entity.onMobSpawn = function(mob)
end

entity.onMobDeath = function(mob, player, optParams)
end

return entity
