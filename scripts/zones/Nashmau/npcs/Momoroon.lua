-----------------------------------
-- Area: Nashmau
--  NPC: Momoroon
-----------------------------------
---@type TNpcEntity
local entity = {}

entity.onTrigger = function(player, npc)
    player:sendMenu(3)
end

return entity
