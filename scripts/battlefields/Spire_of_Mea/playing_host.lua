-----------------------------------
-- Playing Host
-- Spire of Mea ENM30
-- !addkeyitem 672
-----------------------------------
local spireOfMeaID = zones[xi.zone.SPIRE_OF_MEA]
-----------------------------------

local content = Battlefield:new({
    battlefieldId    = xi.battlefield.id.PLAYING_HOST,
    requiredKeyItems = { xi.keyItem.CENSER_OF_ANIMUS, message = spireOfMeaID.text.FADES_INTO_NOTHINGNESS },
    armouryCrates    =
    {
        spireOfMeaID.mob.ENVIER + 4,
        spireOfMeaID.mob.ENVIER + 9,
        spireOfMeaID.mob.ENVIER + 14,
    },
})

content.groups =
{
    -- Envier
    {
        mobIds =
        {
            { spireOfMeaID.mob.ENVIER      },
            { spireOfMeaID.mob.ENVIER + 5  },
            { spireOfMeaID.mob.ENVIER + 10 },
        },

        superlinkGroup = 1,
        allDeath  = utils.bind(content.handleAllMonstersDefeated, content),
    },

    -- Seether
    {
        mobIds =
        {
            {
                spireOfMeaID.mob.ENVIER + 1,
                spireOfMeaID.mob.ENVIER + 2,
                spireOfMeaID.mob.ENVIER + 3,
            },

            {
                spireOfMeaID.mob.ENVIER + 6,
                spireOfMeaID.mob.ENVIER + 7,
                spireOfMeaID.mob.ENVIER + 8,
            },

            {
                spireOfMeaID.mob.ENVIER + 11,
                spireOfMeaID.mob.ENVIER + 12,
                spireOfMeaID.mob.ENVIER + 13,
            },
        },

        superlinkGroup = 1,
        spawned = false,
    },
}

content.loot =
{
    {
        quantity = 3,
        { itemId = xi.item.NONE,                           weight = 200 },
        { itemId = xi.item.CLUSTER_OF_BURNING_MEMORIES,    weight = 100 },
        { itemId = xi.item.CLUSTER_OF_BITTER_MEMORIES,     weight = 100 },
        { itemId = xi.item.CLUSTER_OF_FLEETING_MEMORIES,   weight = 100 },
        { itemId = xi.item.CLUSTER_OF_PROFANE_MEMORIES,    weight = 100 },
        { itemId = xi.item.CLUSTER_OF_STARTLING_MEMORIES,  weight = 100 },
        { itemId = xi.item.CLUSTER_OF_SOMBER_MEMORIES,     weight = 100 },
        { itemId = xi.item.CLUSTER_OF_RADIANT_MEMORIES,    weight = 100 },
        { itemId = xi.item.CLUSTER_OF_MALEVOLENT_MEMORIES, weight = 100 },
    },

    {
        { itemId = xi.item.NONE,                           weight = 500 },
        { itemId = xi.item.SOLEMN_VISION,                  weight = 100 },
        { itemId = xi.item.VALIANT_VISION,                 weight = 100 },
        { itemId = xi.item.PRETENTIOUS_VISION,             weight = 100 },
        { itemId = xi.item.MALICIOUS_VISION,               weight = 100 },
        { itemId = xi.item.PRISTINE_VISION,                 weight = 100 },
    },
}

return content:register()
