-----------------------------------
-- Area: Cloister of Storms
-- BCNM: Sugar Coated Directive (ASA-4)
-----------------------------------
local cloisterOfStormsID = zones[xi.zone.CLOISTER_OF_STORMS]
-----------------------------------

local content = BattlefieldMission:new({
    battlefieldId    = xi.battlefield.id.SUGAR_COATED_DIRECTIVE_CLOISTER_OF_STORMS,

    requiredKeyItems = { xi.keyItem.DOMINAS_VIOLET_SEAL, keep = true },
    missionArea      = xi.mission.log_id.ASA,
    mission          = xi.mission.id.asa.SUGAR_COATED_DIRECTIVE,
    requiredVar      = 'Mission[11][3]Ramuh',
    requiredValue    = 1,

})

content.groups =
{
    {
        mobIds =
        {
            { cloisterOfStormsID.mob.RAMUH_PRIME_ASA     },
            { cloisterOfStormsID.mob.RAMUH_PRIME_ASA + 1 },
            { cloisterOfStormsID.mob.RAMUH_PRIME_ASA + 2 },
        },

        allDeath = function(battlefield, mob)
            battlefield:setStatus(xi.battlefield.status.WON)
        end,
    },
}

return content:register()
