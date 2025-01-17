-----------------------------------
-- func: stagger <mobid-optional>
-- desc: Despawns the given mob <t> or mobID)
-----------------------------------
---@type TCommand
local commandObj = {}

commandObj.cmdprops =
{
    permission = 1,
    parameters = 'i'
}

local function error(player, msg)
    player:printToPlayer(msg)
    player:printToPlayer('!stagger (mobID)')
end

commandObj.onTrigger = function(player, mobId)
    -- validate mobId
    local targ
    if mobId == nil then
        targ = player:getCursorTarget()
        if targ == nil or not targ:isMob() then
            error(player, 'You must either provide a mobID or target a mob.')
            return
        end
    else
        targ = GetMobByID(mobId)
        if targ == nil then
            error(player, 'Invalid mobID.')
            return
        end
    end
    local zone = player:getZoneName()
    if string.find(zone, "^Dynamis") then
        for i = xi.ki.CRIMSON_GRANULES_OF_TIME, xi.ki.OBSIDIAN_GRANULES_OF_TIME do
            if not player:hasKeyItem(i) then
                npcUtil.giveKeyItem(player, i)
            end
        end
        xi.dynamis.procMonster(targ, player)
        player:printToPlayer(string.format('Staggered %s %i.', targ:getName(), targ:getID()))
    elseif string.find(zone, "^Abyssea") then
        xi.abyssea.procMonster(targ, player, xi.abyssea.triggerType.BLUE)
        xi.abyssea.procMonster(targ, player, xi.abyssea.triggerType.RED)
        xi.abyssea.procMonster(targ, player, xi.abyssea.triggerType.YELLOW)
        player:printToPlayer(string.format('Staggered %s %i.', targ:getName(), targ:getID()))
    end
end

return commandObj
