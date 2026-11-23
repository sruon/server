-----------------------------------
-- Area: The_Garden_of_RuHmet
-- Name: When Angels Fall
-----------------------------------
local ruHmetID = zones[xi.zone.THE_GARDEN_OF_RUHMET]
-----------------------------------

local content = BattlefieldMission:new({
    battlefieldId = xi.battlefield.id.WHEN_ANGELS_FALL,
    missionArea   = xi.mission.log_id.COP,
    mission       = xi.mission.id.cop.WHEN_ANGELS_FALL,
    requiredVar   = 'Mission[6][828]Status',
    requiredValue = 4,

})

function content:onEventFinishWin(player, csid, option, npc)
    player:setPos(420, 0, 445, 192)
end

content.groups =
{
    {
        mobIds =
        {
            ruHmetID.mob.IXZDEI_RDM,
            ruHmetID.mob.IXZDEI_RDM + 1,
            ruHmetID.mob.IXZDEI_RDM + 2,
            ruHmetID.mob.IXZDEI_RDM + 3,
        },

        superlink = true,
        allDeath  = function(battlefield, mob)
            battlefield:setStatus(xi.battlefield.status.WON)
        end,
    },
}

return content:register()
