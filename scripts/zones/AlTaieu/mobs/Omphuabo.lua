-----------------------------------
-- Area: Al'Taieu
--  Mob: Om'phuabo
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
