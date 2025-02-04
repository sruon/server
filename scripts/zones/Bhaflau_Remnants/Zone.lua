-----------------------------------
-- Zone: Bhaflau_Remnants
-----------------------------------
---@type TZone
local zoneObject = {}

zoneObject.onInitialize = function(zone)
    zone:registerTriggerArea(1,   340, 4, -420, 0, 0, 0)
    zone:registerTriggerArea(2,   260, 4,  300, 0, 0, 0)
    zone:registerTriggerArea(3,   300, 4,   60, 0, 0, 0)
    zone:registerTriggerArea(4,   420, 4,  300, 0, 0, 0)
    zone:registerTriggerArea(5,   380, 4,   60, 0, 0, 0)
    zone:registerTriggerArea(6,  -460, 4, -500, 0, 0, 0)
    zone:registerTriggerArea(7,  -220, 4, -500, 0, 0, 0)
    zone:registerTriggerArea(8,  -340, 4,   60, 0, 0, 0)
    zone:registerTriggerArea(9,  -380, 4,  380, 0, 0, 0)
    zone:registerTriggerArea(10, -300, 4,  380, 0, 0, 0)
end

zoneObject.onInstanceZoneIn = function(player, instance)
    if player:getInstance() == nil then
        player:setPos(620, 0, -260.640, 72, 72)
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
