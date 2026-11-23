-----------------------------------
-- Area: Stellar Fulcrum
-- Name: ZM8 Return to Delkfutt's Tower
-- !pos -520 -4 17 179
-----------------------------------
local stellarFulcrumID = zones[xi.zone.STELLAR_FULCRUM]
-----------------------------------

local content = BattlefieldMission:new({
    battlefieldId         = xi.battlefield.id.RETURN_TO_DELKFUTTS_TOWER,
    missionArea           = xi.mission.log_id.ZILART,
    mission               = xi.mission.id.zilart.RETURN_TO_DELKFUTTS_TOWER,
    missionStatusArea     = xi.mission.log_id.ZILART,
    requiredMissionStatus = 1,
    title                 = xi.title.DESTROYER_OF_ANTIQUITY,
})

content.groups =
{
    {
        mobIds =
        {
            { stellarFulcrumID.mob.KAMLANAUT     },
            { stellarFulcrumID.mob.KAMLANAUT + 1 },
            { stellarFulcrumID.mob.KAMLANAUT + 2 },
        },

        allDeath = function(battlefield, mob)
            battlefield:setStatus(xi.battlefield.status.WON)
        end,
    },
}

return content:register()
