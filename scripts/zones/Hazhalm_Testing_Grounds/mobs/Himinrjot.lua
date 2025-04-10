-----------------------------------
-- Area: Hazhalm Testing Grounds
--   NM: Himinrjot (Einherjar)
-----------------------------------
mixins =
{
    require('scripts/mixins/draw_in'),
}
-----------------------------------
-- Does not auto-attack, instead uses a text-less Snort that resets hate to some degree
-- Regular Buffalo abilities
-- TODO: High defense, stronger in the front
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    xi.einherjar.onBossInitialize(mob)
    mob:addImmunity(xi.immunity.BIND)
    mob:addImmunity(xi.immunity.GRAVITY)

    mob:setMobSkillAttack(2043) -- use snort_aa as its auto attack
end

entity.onMobSpawn = function(mob)
    -- Snort about every 5 seconds
    mob:setDelay(2000)
end

return entity
