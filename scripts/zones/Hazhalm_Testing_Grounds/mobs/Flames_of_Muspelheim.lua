-----------------------------------
-- Area: Hazhalm Testing Grounds
--   Mob: Flames of Muspelheim (Einherjar)
-------------------------------------
---@type TMobEntity
local entity = {}

entity.onMobFight = function(mod)
    -- TODO: Cluster self destruct mechanic
    -- First: <66%, Second: <32%, Last: <20%
    -- All bombs can explode at once below 20%
    -- mob:useMobAbility(571) -- self_destruct
end

return entity
