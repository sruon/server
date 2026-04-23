-----------------------------------
-- Fetichism
-- Bastok M1-3
-----------------------------------
-- !addmission 1 2
-- Argus   : !pos 132.157 7.496 -2.187 236
-- Cleades : !pos -358 -10 -168 235
-- Malduc  : !pos 66.200 -14.999 4.426 237
-- Rashid  : !pos -8.444 -2 -123.575 234
-----------------------------------
local bastokMarketsID = zones[xi.zone.BASTOK_MARKETS]
local bastokMinesID   = zones[xi.zone.BASTOK_MINES]
local metalworksID    = zones[xi.zone.METALWORKS]
local portBastokID    = zones[xi.zone.PORT_BASTOK]
-----------------------------------

local mission = Mission:new(xi.mission.log_id.BASTOK, xi.mission.id.bastok.FETICHISM)

-- npcUtil.completeMission will only award rank if less than player's current rank in
-- the nation.  Rank Points are cleared on rank up, which occurs after setting.
-- TODO: Verify gil reward occurs on repeat.
mission.reward =
{
    gil = 1000,
    rank = 2,
    rankPoints = 200,
}

local handleAcceptMission = function(player, csid, option, npc)
    if option == 2 then
        mission:begin(player)
        player:messageSpecial(zones[player:getZoneID()].text.YOU_ACCEPT_THE_MISSION)
    end
end

local fetichItems = {
    { xi.item.QUADAV_FETICH_HEAD,  1 },
    { xi.item.QUADAV_FETICH_TORSO, 1 },
    { xi.item.QUADAV_FETICH_ARMS,  1 },
    { xi.item.QUADAV_FETICH_LEGS,  1 },
}

local fetichOnFinish = function(player, option, npc)
    return mission:complete(player)
end

local fetichDeclaredTrades =
{
    {
        match   = { items = fetichItems },
        event = {
            id = function(player)
                return player:hasCompletedMission(mission.areaId, mission.missionId) and 1005 or 1008
            end,
            onFinish = fetichOnFinish,
        },
    },
}

mission.sections =
{
    {
        check = function(player, currentMission, missionStatus, vars)
            return currentMission == xi.mission.id.nation.NONE and
                player:getNation() == mission.areaId
        end,

        [xi.zone.BASTOK_MARKETS] =
        {
            onEventFinish =
            {
                [1001] = handleAcceptMission,
            },
        },

        [xi.zone.BASTOK_MINES] =
        {
            onEventFinish =
            {
                [1001] = handleAcceptMission,
            },
        },

        [xi.zone.METALWORKS] =
        {
            onEventFinish =
            {
                [1001] = handleAcceptMission,
            },
        },

        [xi.zone.PORT_BASTOK] =
        {
            onEventFinish =
            {
                [1001] = handleAcceptMission,
            },
        },
    },

    -- Handles both first time and repeated completions of this mission.  Should there be future findings
    -- that show differences between the two, this should be separated into different sections.
    {
        check = function(player, currentMission, missionStatus, vars)
            return currentMission == mission.missionId
        end,

        [xi.zone.BASTOK_MARKETS] =
        {
            ['Cleades'] =
            {
                declaredTrades = fetichDeclaredTrades,
                onTrigger      = mission:messageSpecial(bastokMarketsID.text.ORIGINAL_MISSION_OFFSET + 6),
            },
        },

        [xi.zone.BASTOK_MINES] =
        {
            ['Rashid'] =
            {
                declaredTrades = fetichDeclaredTrades,
                onTrigger      = mission:messageSpecial(bastokMinesID.text.ORIGINAL_MISSION_OFFSET + 6),
            },
        },

        [xi.zone.METALWORKS] =
        {
            ['Malduc'] =
            {
                declaredTrades = fetichDeclaredTrades,
                onTrigger      = mission:messageSpecial(metalworksID.text.ORIGINAL_MISSION_OFFSET + 6),
            },
        },

        [xi.zone.PORT_BASTOK] =
        {
            ['Argus'] =
            {
                declaredTrades = fetichDeclaredTrades,
                onTrigger      = mission:messageSpecial(portBastokID.text.ORIGINAL_MISSION_OFFSET + 6),
            },
        },
    },
}

return mission
