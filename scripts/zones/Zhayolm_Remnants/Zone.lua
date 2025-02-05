-----------------------------------
-- Zone: Zhayolm_Remnants
-----------------------------------
---@type TZone
local zoneObject = {}

zoneObject.onInitialize = function(zone)
    zone:registerTriggerArea(1,   420, 4, -340, 0, 0, 0)
    zone:registerTriggerArea(2,   420, 4, -500, 0, 0, 0)
    zone:registerTriggerArea(3,   260, 4, -500, 0, 0, 0)
    zone:registerTriggerArea(4,   260, 4, -340, 0, 0, 0)
    zone:registerTriggerArea(5,   340, 4,  -60, 0, 0, 0)
    zone:registerTriggerArea(6,   340, 4,  420, 0, 0, 0)
    zone:registerTriggerArea(7,   340, 4,  500, 0, 0, 0)
    zone:registerTriggerArea(8,  -380, 4, -620, 0, 0, 0)
    zone:registerTriggerArea(9,  -300, 4, -460, 0, 0, 0)
    zone:registerTriggerArea(10, -340, 4, -100, 0, 0, 0)
    zone:registerTriggerArea(11, -340, 4,  140, 0, 0, 0)
    zone:registerTriggerArea(12, -380, 4,  500, 0, 0, 0)
    zone:registerTriggerArea(13, -380, 4,  500, 0, 0, 0)
end

zoneObject.onZoneIn = function(player, prevZone)
    local cs = -1

    return cs
end

zoneObject.onInstanceZoneIn = function(player, instance)
    if player:getInstance() == nil then
        player:setPos(-580, 0, -433, 64, xi.zone.ALZADAAL_UNDERSEA_RUINS)
        return
    end

    local pos = player:getPos()
    if pos.x == 0 and pos.y == 0 and pos.z == 0 then
        local entrypos = instance:getEntryPos()
        player:setPos(entrypos.x, entrypos.y, entrypos.z, entrypos.rot)
        player:startEvent(101)
    end
end

zoneObject.onTriggerAreaEnter = function(player, triggerArea)
end

zoneObject.onEventUpdate = function(player, csid, option, npc)
end

zoneObject.onEventFinish = function(player, csid, option, npc)
end

zoneObject.onInstanceLoadFailed = function()
    return xi.zone.ALZADAAL_UNDERSEA_RUINS
end

return zoneObject
