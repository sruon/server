-----------------------------------
-- Area: Al'Taieu
--  Mob: Ul'phuabo
-----------------------------------
mixins =
{
    require('scripts/mixins/special_roam')({
        idleAnim    = 5,
        fightAnim   = 6,
        returnAnim  = 7,
        returnDelay = 5,
    }),
}
-----------------------------------
---@type TMobEntity
local entity = {}

entity.onMobDeath = function(mob, player, optParams)
end

return entity
