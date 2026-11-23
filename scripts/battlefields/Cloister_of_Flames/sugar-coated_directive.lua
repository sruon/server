-----------------------------------
-- Area: Cloister of Flames
-- BCNM: Sugar Coated Directive (ASA-4)
-----------------------------------
local cloisterOfFlamesID = zones[xi.zone.CLOISTER_OF_FLAMES]
-----------------------------------

local content = BattlefieldMission:new({
    battlefieldId    = xi.battlefield.id.SUGAR_COATED_DIRECTIVE_CLOISTER_OF_FLAMES,

    requiredKeyItems = { xi.keyItem.DOMINAS_SCARLET_SEAL, keep = true },
    missionArea      = xi.mission.log_id.ASA,
    mission          = xi.mission.id.asa.SUGAR_COATED_DIRECTIVE,
    requiredVar      = 'Mission[11][3]Ifrit',
    requiredValue    = 1,

})

content.groups =
{
    {
        mobIds =
        {
            { cloisterOfFlamesID.mob.IFRIT_PRIME_ASA     },
            { cloisterOfFlamesID.mob.IFRIT_PRIME_ASA + 1 },
            { cloisterOfFlamesID.mob.IFRIT_PRIME_ASA + 2 },
        },

        allDeath = function(battlefield, mob)
            battlefield:setStatus(xi.battlefield.status.WON)
        end,
    },
}

return content:register()
