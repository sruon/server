-----------------------------------
-- Area: Hazhalm Testing Grounds
--   NM: Morbol Emperor (Einherjar)
-----------------------------------
mixins =
{
    require('scripts/mixins/draw_in'),
}
-----------------------------------
-- Regular Morbol moves + Vampiric Root
-- Wikis claim this mob follows a pattern on TP moves but captures
-- do not show such behavior. May have been nerfed since then.
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    xi.einherjar.onBossInitialize(mob)
end

return entity
