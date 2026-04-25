-----------------------------------
-- Area: Port Jeuno
--  NPC: Guddal
-- Starts and Finishes Quest: Kazham Airship Pass (This quest does not appear in your quest log) -- Becouse it isn't.
-- !pos -14 8 44 246
-----------------------------------
local ID = zones[xi.zone.PORT_JEUNO]
-----------------------------------
---@type TNpcEntity
local entity = {}

entity.declaredTrades =
{
    {
        match =
        {
            items =
            {
                { xi.item.GHELSBA_CHEST_KEY,    1 },
                { xi.item.PALBOROUGH_CHEST_KEY, 1 },
                { xi.item.GIDDEUS_CHEST_KEY,    1 },
            },
        },
        acceptIf = function(player)
            return not player:hasKeyItem(xi.ki.AIRSHIP_PASS_FOR_KAZHAM)
        end,

        event =
        {
            id = 301,
            onFinish = function(player)
                npcUtil.giveKeyItem(player, xi.ki.AIRSHIP_PASS_FOR_KAZHAM)
                return true
            end,
        },
    },
}

entity.onTrade = function(player, npc, trade)
    -- Fallback for an offer that the declarative trade rejected.
    if not player:hasKeyItem(xi.ki.AIRSHIP_PASS_FOR_KAZHAM) then
        player:startEvent(302)
    end
end

entity.onTrigger = function(player, npc)
    if not player:hasKeyItem(xi.ki.AIRSHIP_PASS_FOR_KAZHAM) then
        player:startEvent(300)
    else
        player:startEvent(300, 0, 0, 0, 0, 0, 6)
    end
end

entity.onEventUpdate = function(player, csid, option, npc)
    if csid == 300 and option == 99 then
        if player:delGil(148000) then
            player:addKeyItem(xi.ki.AIRSHIP_PASS_FOR_KAZHAM)
            player:updateEvent(0, 1)
        end
    end
end

entity.onEventFinish = function(player, csid, option, npc)
    if
        csid == 300 and option == 33 and
        player:hasKeyItem(xi.ki.AIRSHIP_PASS_FOR_KAZHAM)
    then
        player:messageSpecial(ID.text.KEYITEM_OBTAINED, xi.ki.AIRSHIP_PASS_FOR_KAZHAM)
    end
end

return entity
