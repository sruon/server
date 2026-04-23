-----------------------------------
-- Area: La Theine Plateau
--  NPC: Telepoint
-- !pos 420.000 19.104 20.000 102
-----------------------------------
local ID = zones[xi.zone.LA_THEINE_PLATEAU]
-----------------------------------
---@type TNpcEntity
local entity = {}

local function crystalTrade(crystalId)
    return {
        match   = { items = {{ crystalId, 1 }} },
        onSuccess = function(player, npc)
            return npcUtil.giveItem(player, xi.item.FADED_CRYSTAL)
        end,
    }
end

entity.declaredTrades =
{
    crystalTrade(xi.item.FIRE_CRYSTAL),
    crystalTrade(xi.item.ICE_CRYSTAL),
    crystalTrade(xi.item.WIND_CRYSTAL),
    crystalTrade(xi.item.EARTH_CRYSTAL),
    crystalTrade(xi.item.LIGHTNING_CRYSTAL),
    crystalTrade(xi.item.WATER_CRYSTAL),
    crystalTrade(xi.item.LIGHT_CRYSTAL),
    crystalTrade(xi.item.DARK_CRYSTAL),
}

entity.onTrigger = function(player, npc)
    if not player:hasKeyItem(xi.ki.HOLLA_GATE_CRYSTAL) then
        player:startEvent(116)
    else
        player:messageSpecial(ID.text.ALREADY_OBTAINED_TELE)
    end
end

entity.onEventFinish = function(player, csid, option, npc)
    if csid == 116 then
        npcUtil.giveKeyItem(player, xi.ki.HOLLA_GATE_CRYSTAL)
    end
end

return entity
