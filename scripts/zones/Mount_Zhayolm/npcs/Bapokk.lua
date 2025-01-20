-----------------------------------
-- Area: Mount Zhayolm
--  NPC: Bapokk
-- Handles access to Alzadaal Ruins
-- !pos -20 -6 276 61
-----------------------------------
local ID = zones[xi.zone.MOUNT_ZHAYOLM]
-----------------------------------
---@type TNpcEntity
local entity = {}

entity.onTrade = function(player, npc, trade)
    if
        trade:getItemCount() == 1 and
        trade:hasItemQty(xi.item.IMPERIAL_SILVER_PIECE, 1)
    then
        player:tradeComplete()
        player:setPos(-20, 3.7, 316, 198) -- using the pos method until the problem below is fixed
        -- player:startEvent(163) -- << this CS goes black at the end, never fades in
    end
end

entity.onTrigger = function(player, npc)
    if player:getZPos() > 280 then
        player:startEvent(164) -- Ruins -> Zhayolm
    else
        if player:hasKeyItem(xi.ki.CAPTAIN_WILDCAT_BADGE) then -- Zhayolm -> Ruins
            player:messageSpecial(ID.text.YOU_HAVE_A_BADGE, xi.ki.CAPTAIN_WILDCAT_BADGE)
            player:setPos(-20, 3.7, 316, 198)
            -- player:startEvent(163)
        else
            player:startEvent(162)
        end
    end
end

entity.onEventUpdate = function(player, csid, option, npc)
end

entity.onEventFinish = function(player, csid, option, npc)
end

return entity
