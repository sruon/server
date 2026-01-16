-----------------------------------
-- Area: Al'Taieu
--  Mob: Ul'hpemde
-----------------------------------
mixins =
{
    require('scripts/mixins/special_roam')({
        idleAnim    = 5,
        fightAnim   = 6,
        returnDelay = 5,
    }),
    require('scripts/mixins/families/hpemde'),
}
-----------------------------------
---@type TMobEntity
local entity = {}

entity.onMobDeath = function(mob, player, optParams)
end

return entity
