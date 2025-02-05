-----------------------------------
-- Zone: Silver_Sea_Remnants
-----------------------------------
---@type TZone
local zoneObject = {}

zoneObject.onInitialize = function(zone)
    zone:registerTriggerArea(1, 305.5, -1, -414.5, 374.5, 1, -345.5) -- Special Fomor room
    zone:registerTriggerArea(2,   340, 4, -300, 0, 0, 0)
    zone:registerTriggerArea(3,   260, 4,  380, 0, 0, 0)
    zone:registerTriggerArea(4,   300, 4,  620, 0, 0, 0)
    zone:registerTriggerArea(5,   420, 4,  380, 0, 0, 0)
    zone:registerTriggerArea(6,   380, 4,  620, 0, 0, 0)
    zone:registerTriggerArea(7,  -460, 4, -300, 0, 0, 0)
    zone:registerTriggerArea(8,  -220, 4, -300, 0, 0, 0)
    zone:registerTriggerArea(9,  -340, 4,  100, 0, 0, 0)
    zone:registerTriggerArea(10, -300, 4,  620, 0, 0, 0)
    zone:registerTriggerArea(11, -380, 4,  620, 0, 0, 0)
end

zoneObject.onInstanceZoneIn = function(player, instance)
    if player:getInstance() == nil then
        player:setPos(580, 0, 500, 192, 72)
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
