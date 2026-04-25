-----------------------------------
-- Smash the Orcish Scouts
-- San d'Oria M1-1
-----------------------------------
-- NOTE: This handles both the first completion and repeated
-- !addmission 0 0
-- Ambrotien  : !pos 93.419 -0.001 -57.347 230
-- Grilau     : !pos -241.987 6.999 57.887 231
-- Endracion  : !pos -110 1 -34 230
-----------------------------------
-- Orcish Axe : ItemID 16656
-----------------------------------
local southernSandoriaID = zones[xi.zone.SOUTHERN_SAN_DORIA]
local northernSandoriaID = zones[xi.zone.NORTHERN_SAN_DORIA]
-----------------------------------

local mission = Mission:new(xi.mission.log_id.SANDORIA, xi.mission.id.sandoria.SMASH_THE_ORCISH_SCOUTS)

mission.reward =
{
    rankPoints = 150,
}

local handleAcceptMission = function(player, csid, option, npc)
    if option == 0 then
        mission:begin(player)
        player:messageSpecial(zones[player:getZoneID()].text.YOU_ACCEPT_THE_MISSION)
    end
end

local function orcishAxeDecl(firstId, repeatId)
    return {
        match = { items = { { xi.item.ORCISH_AXE, 1 } } },
        event =
        {
            id = function(player)
                return player:hasCompletedMission(mission.areaId, mission.missionId) and repeatId or firstId
            end,

            onFinish = function(player)
                return mission:complete(player)
            end,
        },
    }
end

mission.sections =
{
    -- Player is offered this mission for the first time
    {
        check = function(player, currentMission, missionStatus, vars)
            return currentMission == xi.mission.id.nation.NONE and
                player:getNation() == mission.areaId and
                not player:hasCompletedMission(mission.areaId, mission.missionId)
        end,

        [xi.zone.SOUTHERN_SAN_DORIA] =
        {
            ['Ambrotien'] = mission:progressEvent(2000),
            ['Endracion'] = mission:progressEvent(1000),

            onEventFinish =
            {
                [1000] = handleAcceptMission,
                [2000] = handleAcceptMission,
            },
        },

        [xi.zone.NORTHERN_SAN_DORIA] =
        {
            ['Grilau'] = mission:progressEvent(1000),

            onEventFinish =
            {
                [1000] = handleAcceptMission,
            },
        },
    },

    -- Player is repeating this Mission
    {
        check = function(player, currentMission, missionStatus, vars)
            return currentMission == xi.mission.id.nation.NONE and
                player:getNation() == mission.areaId and
                player:hasCompletedMission(mission.areaId, mission.missionId)
        end,

        [xi.zone.SOUTHERN_SAN_DORIA] =
        {
            onEventFinish =
            {
                [1009] = handleAcceptMission,
                [2009] = handleAcceptMission,
            },
        },

        [xi.zone.NORTHERN_SAN_DORIA] =
        {
            onEventFinish =
            {
                [1009] = handleAcceptMission,
            },
        },
    },

    -- Mission has been accepted; This section handles logic for both first time
    -- and repeat completion
    {
        check = function(player, currentMission, missionStatus, vars)
            return currentMission == mission.missionId
        end,

        [xi.zone.SOUTHERN_SAN_DORIA] =
        {
            ['Ambrotien'] =
            {
                onTrigger      = mission:messageText(southernSandoriaID.text.ORIGINAL_MISSION_OFFSET),
                declaredTrades = { orcishAxeDecl(2020, 2002) },
            },

            ['Endracion'] =
            {
                onTrigger      = mission:messageText(southernSandoriaID.text.ORIGINAL_MISSION_OFFSET),
                declaredTrades = { orcishAxeDecl(1020, 1002) },
            },
        },

        [xi.zone.NORTHERN_SAN_DORIA] =
        {
            ['Grilau'] =
            {
                onTrigger      = mission:messageText(northernSandoriaID.text.ORIGINAL_MISSION_OFFSET),
                declaredTrades = { orcishAxeDecl(1020, 1002) },
            },
        },
    },
}

return mission
