-----------------------------------
-- Area: Hazhalm Testing Grounds
--   NM: Stoorworm (Einherjar)
-----------------------------------
mixins =
{
    require('scripts/mixins/families/hydra'),
    require('scripts/mixins/job_special'),
    require('scripts/mixins/draw_in'),
}
-----------------------------------
-- Regular Hydra TP moves
-- Can use Mighty Strikes
-- Heads regrow 60-90 seconds after being destroyed
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    xi.einherjar.onBossInitialize(mob)
    mob:addImmunity(xi.immunity.BIND)
    mob:addImmunity(xi.immunity.PARALYZE)
    mob:addImmunity(xi.immunity.GRAVITY)
end

entity.onMobSpawn = function(mob)
    mob:setLocalVar('headRegrowMin', 60)
    mob:setLocalVar('headRegrowMax', 90)
end

return entity
