-----------------------------------
-- You Are What You Eat
-- Spire of Dem ENM30
-- !addkeyitem 671
-----------------------------------
local spireOfDemID = zones[xi.zone.SPIRE_OF_DEM]
-----------------------------------

local content = Battlefield:new({
    battlefieldId    = xi.battlefield.id.YOU_ARE_WHAT_YOU_EAT,
    requiredKeyItems = { xi.keyItem.CENSER_OF_ANTIPATHY, message = spireOfDemID.text.FADES_INTO_NOTHINGNESS },
})

content.groups =
{
    {
        mobs      = { 'Ingester' },
        superlinkGroup = 1,
        allDeath  = utils.bind(content.handleAllMonstersDefeated, content),
    },

    {
        mobs      = { 'Neoingester', 'Neogorger', 'Neosatiator', 'Wanderer_enm' },
        superlinkGroup = 1,
        spawned  = false,
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
        { itemId = xi.item.VIOLENT_VISION,                 weight = 100 },
        { itemId = xi.item.PAINFUL_VISION,                 weight = 100 },
        { itemId = xi.item.TIMOROUS_VISION,                weight = 100 },
        { itemId = xi.item.BRILLIANT_VISION,               weight = 100 },
        { itemId = xi.item.VENERABLE_VISION,               weight = 100 },
    },
}

return content:register()
