-----------------------------------
-- Brothers
-- Bearclaw Pinnacle ENM, Zephyr Fan
-- !addkeyitem ZEPHYR_FAN
-- !pos 121 -171 758 6
-----------------------------------
local ID = zones[xi.zone.BEARCLAW_PINNACLE]
-----------------------------------

local content = Battlefield:new({
    battlefieldId    = xi.battlefield.id.BROTHERS,
    requiredKeyItems = { xi.keyItem.ZEPHYR_FAN, message = ID.text.ZEPHYR_RIPS },
})

content:addEssentialMobs({ 'Eldertaur', 'Mindertaur' })

content.loot =
{
    {
        { itemId = xi.item.NONE,                     weight = 9750 },
        { itemId = xi.item.CLOUD_EVOKER,             weight =  250 },
    },

    {
        { itemId = xi.item.NONE,                     weight = 2500 },
        { itemId = xi.item.PIECE_OF_CASSIA_LUMBER,   weight = 2500 },
        { itemId = xi.item.SQUARE_OF_ELTORO_LEATHER, weight = 2500 },
        { itemId = xi.item.DRAGON_BONE,              weight = 2500 },
    },

    {
        quantity = 2,
        { itemId = xi.item.NONE,                     weight = 5500 },
        { itemId = xi.item.SCOUTERS_ROPE,            weight =  800 },
        { itemId = xi.item.HEDGEHOG_BOMB,            weight =  700 },
        { itemId = xi.item.MARTIAL_ANELACE,          weight =  800 },
        { itemId = xi.item.MARTIAL_LANCE,            weight =  800 },
        { itemId = xi.item.SCROLL_OF_RAISE_III,      weight = 1400 },
    },
}

return content:register()
