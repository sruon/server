-----------------------------------
-- Area: La Vaule [S]
-- BCNM: The Blood-bathed Crown
-----------------------------------
local ID = zones[xi.zone.LA_VAULE_S]
-----------------------------------
ID.text.KEYITEM_LOST                   = 6394
ID.text.TIME_IN_THE_BATTLEFIELD_IS_UP  = 7692
ID.text.PARTY_MEMBERS_HAVE_FALLEN      = 7997
ID.text.MEMBERS_OF_YOUR_ALLIANCE       = 7998
ID.text.TIME_LIMIT_FOR_THIS_BATTLE_IS  = 8000
ID.text.THE_PARTY_WILL_BE_REMOVED      = 8043
ID.text.ENTERING_THE_BATTLEFIELD_FOR   = 8065
ID.mob.BLOODCROWN_BRRADHOD = 17125684

local content = Battlefield:new({
    zoneId           = xi.zone.LA_VAULE_S,
    battlefieldId    = 3001,
    maxPlayers       = 18,
    allowTrusts      = false,
    levelCap         = 75,
    allowSubjob      = true,
    timeLimit        = utils.minutes(30),
    index            = 2,
    entryNpc         = '_2d1',
    exitNpcs         = { '_2d3', '_2d5', '_2d7' },
    requiredKeyItems =
    {
        xi.ki.IMPERIAL_LINEAGE_CHAPTER_I,
        xi.ki.IMPERIAL_LINEAGE_CHAPTER_II,
        xi.ki.IMPERIAL_LINEAGE_CHAPTER_III,
        xi.ki.IMPERIAL_LINEAGE_CHAPTER_IV,
        xi.ki.IMPERIAL_LINEAGE_CHAPTER_V,
        xi.ki.IMPERIAL_LINEAGE_CHAPTER_VI,
        xi.ki.IMPERIAL_LINEAGE_CHAPTER_VII,
        xi.ki.IMPERIAL_LINEAGE_CHAPTER_VIII,
        message = 8052, -- Imperial Lineage” chapters disappear!
    },

    experimental     = false,
    armouryCrates    =
    {
        ID.mob.BLOODCROWN_BRRADHOD + 5,
    }
})

content.groups =
{
    {
        mobIds =
        {
            {
                ID.mob.BLOODCROWN_BRRADHOD,
                ID.mob.BLOODCROWN_BRRADHOD + 1,
                ID.mob.BLOODCROWN_BRRADHOD + 2,
                ID.mob.BLOODCROWN_BRRADHOD + 3,
                ID.mob.BLOODCROWN_BRRADHOD + 4,
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
