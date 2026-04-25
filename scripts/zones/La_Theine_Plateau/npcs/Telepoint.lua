-----------------------------------
-- Area: La Theine Plateau
--  NPC: Telepoint
-- !pos 420.000 19.104 20.000 102
-----------------------------------
local ID = zones[xi.zone.LA_THEINE_PLATEAU]
-----------------------------------
---@type TNpcEntity
local entity = {}

local function giveFadedCrystal(player)
    return npcUtil.giveItem(player, xi.item.FADED_CRYSTAL)
end

entity.declaredTrades =
{
    { match = { items = { { xi.item.FIRE_CRYSTAL,      1 } } }, onSuccess = giveFadedCrystal },
    { match = { items = { { xi.item.ICE_CRYSTAL,       1 } } }, onSuccess = giveFadedCrystal },
    { match = { items = { { xi.item.WIND_CRYSTAL,      1 } } }, onSuccess = giveFadedCrystal },
    { match = { items = { { xi.item.EARTH_CRYSTAL,     1 } } }, onSuccess = giveFadedCrystal },
    { match = { items = { { xi.item.LIGHTNING_CRYSTAL, 1 } } }, onSuccess = giveFadedCrystal },
    { match = { items = { { xi.item.WATER_CRYSTAL,     1 } } }, onSuccess = giveFadedCrystal },
    { match = { items = { { xi.item.LIGHT_CRYSTAL,     1 } } }, onSuccess = giveFadedCrystal },
    { match = { items = { { xi.item.DARK_CRYSTAL,      1 } } }, onSuccess = giveFadedCrystal },
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
