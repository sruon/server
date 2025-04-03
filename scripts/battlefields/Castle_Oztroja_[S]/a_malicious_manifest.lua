-----------------------------------
-- A Malicious Manifest
-- Stronghold Battlefield
-----------------------------------
local ID = zones[xi.zone.CASTLE_OZTROJA_S]
-----------------------------------
ID.text.PARTY_MEMBERS_ARE_ENGAGED     = ID.text.CAMPAIGN_RESULTS_TALLIED + 108
ID.text.NO_BATTLEFIELD_ENTRY          = ID.text.CAMPAIGN_RESULTS_TALLIED + 112
ID.text.TIME_IN_THE_BATTLEFIELD_IS_UP = ID.text.PARTY_MEMBERS_HAVE_FALLEN - 342
ID.text.PARTY_MEMBERS_HAVE_FALLEN     = ID.text.PARTY_MEMBERS_HAVE_FALLEN + 3
ID.text.MEMBERS_OF_YOUR_ALLIANCE      = ID.text.PARTY_MEMBERS_HAVE_FALLEN - 35
ID.text.TIME_LIMIT_FOR_THIS_BATTLE_IS = ID.text.PARTY_MEMBERS_HAVE_FALLEN - 33
ID.text.ENTERING_THE_BATTLEFIELD_FOR  = ID.text.THE_PARTY_WILL_BE_REMOVED + 25
ID.mob.TZEE_XICU_THE_MANIFEST         = 17183062

local content = Battlefield:new({
    zoneId           = xi.zone.CASTLE_OZTROJA_S,
    battlefieldId    = 3000,
    maxPlayers       = 18,
    allowTrusts      = false,
    levelCap         = 75,
    allowSubjob      = true,
    timeLimit        = utils.minutes(30),
    index            = 1,
    grantXP          = 2000,
    entryNpc         = '_2r8',
    exitNpc          = { '_2r8' },
    requiredKeyItems =
    {
        xi.ki.HABALOS_ECLOGUE_VERSE_I,
        xi.ki.HABALOS_ECLOGUE_VERSE_II,
        xi.ki.HABALOS_ECLOGUE_VERSE_III,
        xi.ki.HABALOS_ECLOGUE_VERSE_IV,
        xi.ki.HABALOS_ECLOGUE_VERSE_V,
        xi.ki.HABALOS_ECLOGUE_VERSE_VI,
        xi.ki.HABALOS_ECLOGUE_VERSE_VII,
        xi.ki.HABALOS_ECLOGUE_VERSE_VIII,
        message = ID.text.THE_PARTY_WILL_BE_REMOVED + 12,
    },
    experimental     = false,
    armouryCrates    =
    {
        ID.mob.TZEE_XICU_THE_MANIFEST + 5,
    }
})

content.groups =
{
    {
        mobIds =
        {
            { 
                ID.mob.TZEE_XICU_THE_MANIFEST, 
                ID.mob.TZEE_XICU_THE_MANIFEST + 1,
                ID.mob.TZEE_XICU_THE_MANIFEST + 2,
                ID.mob.TZEE_XICU_THE_MANIFEST + 3,
                ID.mob.TZEE_XICU_THE_MANIFEST + 4,
            },
        },

        superlink = true,
        allDeath  = utils.bind(content.handleAllMonstersDefeated, content),
    },
}

content.loot =
{
    {
        { item = xi.item.NONE, weight = 1000 },  -- Square of Nothing (100% Drop Rate)
    },
}

return content:register()
