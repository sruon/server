-----------------------------------
-- Area: Hazhalm Testing Grounds
--   NM: Gorgimera (Einherjar)
-----------------------------------
mixins =
{
    require('scripts/mixins/draw_in'),
    require('scripts/mixins/families/khimaira')
}
-----------------------------------
-- Regular Khimaira moves
-- No known special mechanics
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    xi.einherjar.onBossInitialize(mob)
end

return entity
