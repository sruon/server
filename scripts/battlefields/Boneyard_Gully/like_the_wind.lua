-----------------------------------
-- Like the Wind
-- Boneyard Gully ENM, Miasma Filter
-- !addkeyitem MIASMA_FILTER
-----------------------------------

local content = Battlefield:new({
    battlefieldId    = xi.battlefield.id.LIKE_THE_WIND,
    requiredKeyItems = { xi.keyItem.MIASMA_FILTER },


})

-- TODO: Race Runner has a constant movement mechanic and different hate rules that need
-- to be implemented.  See: https://ffxiclopedia.fandom.com/wiki/Like_the_Wind

content:addEssentialMobs({ 'Race_Runner' })

content.loot =
{
    {
        { itemId = xi.item.NONE,                   weight = 1400 }, -- nothing
        { itemId = xi.item.POT_OF_VIRIDIAN_URUSHI, weight = 3100 }, -- Viridian Urushi
        { itemId = xi.item.SQUARE_OF_GALATEIA,     weight = 2400 }, -- Square of Galateia
        { itemId = xi.item.SQUARE_OF_KEJUSU_SATIN, weight = 3100 }, -- Kejusu Satin
    },

    {
        { itemId = xi.item.NONE,         weight = 9750 }, -- nothing
        { itemId = xi.item.CLOUD_EVOKER, weight =  250 }, -- Cloud Evoker
    },

    {
        { itemId = xi.item.NONE,                    weight = 3800 }, -- nothing
        { itemId = xi.item.MANEATER,                weight = 1380 }, -- Maneater
        { itemId = xi.item.WAGH_BAGHNAKHS,          weight = 1720 }, -- Wagh Baghnakhs
        { itemId = xi.item.ONIMARU,                 weight = 1380 }, -- Onimaru
        { itemId = xi.item.SCROLL_OF_ARMYS_PAEON_V, weight = 1720 }, -- Army's Paeon V
    },

    {
        { itemId = xi.item.NONE,                    weight = 3800 }, -- nothing
        { itemId = xi.item.MANEATER,                weight = 1380 }, -- Maneater
        { itemId = xi.item.WAGH_BAGHNAKHS,          weight = 1720 }, -- Wagh Baghnakhs
        { itemId = xi.item.ONIMARU,                 weight = 1380 }, -- Onimaru
        { itemId = xi.item.SCROLL_OF_ARMYS_PAEON_V, weight = 1720 }, -- Army's Paeon V
    },
}

return content:register()
