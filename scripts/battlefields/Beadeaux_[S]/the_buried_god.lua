-----------------------------------
-- Area: Beadeaux [S]
-- BCNM: The Buried God
-----------------------------------
local ID = zones[xi.zone.BEADEAUX_S]
-----------------------------------
ID.text.KEYITEM_LOST                  = 6394
ID.text.TIME_IN_THE_BATTLEFIELD_IS_UP = 7591
ID.text.MEMBERS_OF_YOUR_ALLIANCE      = 7898
ID.text.TIME_LIMIT_FOR_THIS_BATTLE_IS = 7900
ID.text.PARTY_MEMBERS_HAVE_FALLEN     = 7936
ID.text.THE_PARTY_WILL_BE_REMOVED     = 7943
ID.text.ENTERING_THE_BATTLEFIELD_FOR  = 7965
ID.mob.ZADHA_ADAMANTKING              = 17154387

local content = Battlefield:new({
    zoneId                = xi.zone.BEADEAUX_S,
    battlefieldId         = 3002,
    maxPlayers            = 18,
    allowTrusts           = false,
    levelCap              = 75,
    allowSubjob           = true,
    timeLimit             = utils.minutes(30),
    index                 = 1,
    entryNpc              = '_2k5',
    exitNpcs              = { '_2k6', '_2k8', '_2ka' },
    requiredKeyItems =
    {
        xi.ki.THE_WORDS_OF_DONHU_I,
        xi.ki.THE_WORDS_OF_DONHU_II,
        xi.ki.THE_WORDS_OF_DONHU_III,
        xi.ki.THE_WORDS_OF_DONHU_IV,
        xi.ki.THE_WORDS_OF_DONHU_V,
        xi.ki.THE_WORDS_OF_DONHU_VI,
        xi.ki.THE_WORDS_OF_DONHU_VII,
        xi.ki.THE_WORDS_OF_DONHU_VIII,
        message = 7956, -- All of the “Words of Do'Nhu” tablets disappear!
    },
    experimental     = false,
    armouryCrates    =
    {
        ID.mob.ZADHA_ADAMANTKING + 5,
    }
})

content.groups =
{
    {
        mobIds =
        {
            {
                ID.mob.ZADHA_ADAMANTKING,
                ID.mob.ZADHA_ADAMANTKING + 1,
                ID.mob.ZADHA_ADAMANTKING + 2,
                ID.mob.ZADHA_ADAMANTKING + 3,
                ID.mob.ZADHA_ADAMANTKING + 4,
            },
        },

        superlink = true,
        allDeath  = utils.bind(content.handleAllMonstersDefeated, content),
    }}

content.loot =
{
    {
        { item = xi.item.NONE, weight = 1000 },  -- Square of Nothing (100% Drop Rate)
    },
}

return content:register()
