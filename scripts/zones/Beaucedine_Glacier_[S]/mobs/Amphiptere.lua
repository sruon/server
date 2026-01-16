-----------------------------------
-- Area: Beaucedine Glacier [S]
--   NM: Amphiptere
-----------------------------------
mixins =
{
    require('scripts/mixins/special_roam')({
        idleAnim    = 5,
        fightAnim   = 4,
        returnDelay = 5,
    }),
    require('scripts/mixins/families/amphiptere'),
}
-----------------------------------
---@type TMobEntity
local entity = {}

entity.onMobDeath = function(mob, player, optParams)
end

return entity
