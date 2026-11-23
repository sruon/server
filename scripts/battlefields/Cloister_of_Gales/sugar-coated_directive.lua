-----------------------------------
-- Area: Cloister of Gales
-- BCNM: Sugar Coated Directive (ASA-4)
-----------------------------------
local cloisterOfGalesID = zones[xi.zone.CLOISTER_OF_GALES]
-----------------------------------

local content = BattlefieldMission:new({
    battlefieldId    = xi.battlefield.id.SUGAR_COATED_DIRECTIVE_CLOISTER_OF_GALES,

    requiredKeyItems = { xi.keyItem.DOMINAS_EMERALD_SEAL, keep = true },
    missionArea      = xi.mission.log_id.ASA,
    mission          = xi.mission.id.asa.SUGAR_COATED_DIRECTIVE,
    requiredVar      = 'Mission[11][3]Garuda',
    requiredValue    = 1,

})

content.groups =
{
    {
        mobIds =
        {
            { cloisterOfGalesID.mob.GARUDA_PRIME_ASA     },
            { cloisterOfGalesID.mob.GARUDA_PRIME_ASA + 1 },
            { cloisterOfGalesID.mob.GARUDA_PRIME_ASA + 2 },
        },

        allDeath = function(battlefield, mob)
            battlefield:setStatus(xi.battlefield.status.WON)
        end,
    },
}

return content:register()
