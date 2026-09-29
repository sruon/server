-----------------------------------
-- func: pathdebug [on|freeze|off|nodes] [model]
-- desc: Privately display the targeted mob's active path waypoints.
-----------------------------------
---@type TCommand
local commandObj = {}

commandObj.cmdprops =
{
    permission = 1,
    parameters = 'si',
}

commandObj.onTrigger = function(player, mode, model)
    if mode == 'off' then
        player:printToPlayer(player:pathDebug(nil, false, 0))
        return
    end

    if mode ~= nil and mode ~= 'freeze' and mode ~= 'on' and mode ~= 'nodes' then
        player:printToPlayer('Usage: !pathdebug [on|freeze|off|nodes] [model ID]')
        return
    end

    if model ~= nil and (model < 1 or model > 65535) then
        player:printToPlayer('Model ID must be between 1 and 65535.')
        return
    end

    if mode == 'nodes' then
        -- dofile rereads the file, so edits show without a restart
        local path = string.format('scripts/zones/%s/pathNodes.lua', player:getZoneName())
        local ok, points = pcall(dofile, path)
        if not ok then
            player:printToPlayer(string.format('Could not load %s: %s', path, tostring(points)))
            return
        end

        player:printToPlayer(player:pathDebugPoints(points, model))
        return
    end

    local target = player:getCursorTarget()
    if target == nil or not target:isMob() then
        player:printToPlayer('Select a living mob first.')
        return
    end

    player:printToPlayer(player:pathDebug(target, mode == 'freeze', model or 0))
end

return commandObj
