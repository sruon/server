-----------------------------------
-- Area: La Vaule [S]
-- BCNM: Purple, The New Black (WOTG07)
--  Mob: Galarhigg (17125681 or 17125682 or 17125683)
-----------------------------------
local laVauleID = zones[xi.zone.LA_VAULE_S]
-----------------------------------

local content = BattlefieldMission:new({
    battlefieldId         = xi.battlefield.id.PURPLE_THE_NEW_BLACK,
    missionArea           = xi.mission.log_id.WOTG,
    mission               = xi.mission.id.wotg.PURPLE_THE_NEW_BLACK,
    missionStatusArea     = xi.mission.log_id.WOTG,
    requiredMissionStatus = 1,
})

content.groups =
{
    {
        mobIds =
        {
            { laVauleID.mob.GALARHIGG     },
            { laVauleID.mob.GALARHIGG + 1 },
            { laVauleID.mob.GALARHIGG + 2 },
        },

        allDeath = function(battlefield, mob)
            battlefield:setStatus(xi.battlefield.status.WON)
        end,
    },
}

return content:register()
