-----------------------------------
-- Area: Hazhalm Testing Grounds
--   NM: Odin (Einherjar)
-----------------------------------
mixins =
{
    require('scripts/mixins/draw_in'),
}
-----------------------------------
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    -- Common Einherjar resistances
    xi.einherjar.onBossInitialize(mob)

    -- Odin specific resistances
    mob:addImmunity(xi.immunity.BLIND)
    mob:addImmunity(xi.immunity.SILENCE)
end

return entity
