-----------------------------------
-- Area: Hazhalm Testing Grounds
--   NM: Nihhus (Einherjar)
-----------------------------------
mixins =
{
    require('scripts/mixins/draw_in'),
}
-----------------------------------
-- Uses standard Wivre TP moves + Crippling Slam
-- TODO: Unverified/unimplemented claims:
--  - Batterhorn seems to reset hate.
--  - Crippling Slam is only used under 30%
--  - Magic damage is reduced by 35%
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    xi.einherjar.onBossInitialize(mob)
end

return entity
