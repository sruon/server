-----------------------------------
-- Area: Norg
--  NPC: Agatsum
-----------------------------------
---@type TNpcEntity
local entity = {}

entity.onTrigger = function(player, npc)
    player:sendMenu(3)
end

return entity
