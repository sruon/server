-----------------------------------
-- Area: Port Jeuno
--  NPC: Shami
-- Orb Seller (BCNM)
-- !pos -53.9 0 10.8 246
-----------------------------------
---@type TNpcEntity
local entity = {}

local shamiOrbItems =
{
    -- Item ID                    CS, PO, SealID, Cost
    [xi.item.CLOUDY_ORB     ] = {  5,  1,      0,   20 },
    [xi.item.SKY_ORB        ] = {  9,  2,      0,   30 },
    [xi.item.STAR_ORB       ] = {  9,  3,      0,   40 },
    [xi.item.COMET_ORB      ] = {  9,  4,      0,   50 },
    [xi.item.MOON_ORB       ] = {  9,  5,      0,   60 },
    [xi.item.CLOTHO_ORB     ] = {  9,  6,      1,   30 },
    [xi.item.LACHESIS_ORB   ] = {  9,  7,      1,   30 },
    [xi.item.ATROPOS_ORB    ] = {  9,  8,      1,   30 },
    [xi.item.THEMIS_ORB     ] = { 11,  9,      1,   99 },
    [xi.item.PHOBOS_ORB     ] = { 11, 10,      2,   30 },
    [xi.item.DEIMOS_ORB     ] = { 11, 11,      2,   50 },
    [xi.item.ZELOS_ORB      ] = { 11, 12,      3,   30 },
    [xi.item.BIA_ORB        ] = { 11, 13,      3,   50 },
    [xi.item.MICROCOSMIC_ORB] = { 11, 14,      4,   10 },
    [xi.item.MACROCOSMIC_ORB] = { 11, 15,      4,   20 },
}

---@nodiscard
---@param option integer
---@return xi.item?, integer?, integer?
local function convertSealRetrieveOption(option)
    for itemID, sealData in pairs(xi.seals.sealItems) do
        if (option + sealData[2]) % 256 == 0 then
            local sealCount = (option + sealData[2]) / 256 - 1

            return itemID, sealData[1], sealCount
        end
    end

    return nil, nil, nil
end

---@nodiscard
---@param option integer
---@return xi.item?, integer?, integer?
local function getOrbDataFromOption(option)
    for itemID, orbData in pairs(shamiOrbItems) do
        if orbData[2] == option then
            return itemID, orbData[3], orbData[4]
        end
    end

    return nil, nil, nil
end

entity.declaredTrades =
{
    {
        match =
        {
            anyOf =
            {
                xi.item.BEASTMENS_SEAL,
                xi.item.KINDREDS_SEAL,
                xi.item.KINDREDS_CREST,
                xi.item.HIGH_KINDREDS_CREST,
                xi.item.SACRED_KINDREDS_CREST,
            },
        },
        event =
        {
            id     = 321,
            params = function(player, npc, offers)
                local out = { 0, 0, 0, 0, 0, 0 }
                for _, entry in ipairs(offers) do
                    local sealData = xi.seals.sealItems[entry.id]
                    if sealData then
                        local sealId = sealData[1]
                        out[sealId + 2] = bit.lshift(player:getSeals(sealId) + entry.qty, 16)
                    end
                end

                return out
            end,

            onFinish = function(player, option, npc, confirmations)
                for itemId, qty in pairs(confirmations) do
                    local sealId = xi.seals.sealItems[itemId][1]
                    player:addSeals(qty, sealId)
                end

                return true
            end,
        },
    },

    {
        match =
        {
            anyOf =
            {
                xi.item.CLOUDY_ORB,
                xi.item.SKY_ORB,
                xi.item.STAR_ORB,
                xi.item.COMET_ORB,
                xi.item.MOON_ORB,
                xi.item.CLOTHO_ORB,
                xi.item.LACHESIS_ORB,
                xi.item.ATROPOS_ORB,
                xi.item.THEMIS_ORB,
                xi.item.PHOBOS_ORB,
                xi.item.DEIMOS_ORB,
                xi.item.ZELOS_ORB,
                xi.item.BIA_ORB,
                xi.item.MICROCOSMIC_ORB,
                xi.item.MACROCOSMIC_ORB,
            },
        },
        acceptIf = function(player, npc, offers)
            return #offers == 1
        end,

        event =
        {
            id = function(player, npc, offers)
                local orbId = offers[1].id
                -- Cracked orb → generic CS 22; fresh → per-orb intro CS.
                return player:getWornUses(orbId) > 0 and 22 or shamiOrbItems[orbId][1]
            end,

            onFinish = function(player, option, npc, confirmations)
                -- Consume only cracked orbs. Fresh orbs played their
                -- intro CS and stay with the player.
                for orbId in pairs(confirmations) do
                    if player:getWornUses(orbId) > 0 then
                        return true
                    end
                end

                return false
            end,
        },
    },
}

entity.onTrigger = function(player, npc)
    local beastmensSeal       = player:getSeals(0)
    local kindredsSeal        = player:getSeals(1)
    local kindredsCrest       = player:getSeals(2)
    local highKindredsCrest   = player:getSeals(3)
    local sacredKindredsCrest = player:getSeals(4)

    -- TODO: player:startEvent(322, 0, 0, 0, 0, 1, 0, 1) -- First time talking to him WITH beastmen seal in inventory
    if beastmensSeal + kindredsSeal + kindredsCrest + highKindredsCrest + sacredKindredsCrest == 0 then
        player:startEvent(23) -- Standard dialog ?
    else
        local purchasedOrbs = player:getCharVar('ShamiPurchasedOrbs') -- Skips the first longer dialog if the player has already purchased orbs
        player:startEvent(322, (kindredsSeal * 65536) + beastmensSeal, (highKindredsCrest * 65536) + kindredsCrest, sacredKindredsCrest, 0, 1, 0, 0, math.min(purchasedOrbs, 1)) -- Standard dialog with menu
    end
end

entity.onEventFinish = function(player, csid, option, npc)
    -- Retrieving Seals
    if
        option >= 508 and
        option ~= utils.EVENT_CANCELLED_OPTION
    then
        local itemID, sealID, retrievedSealCount = convertSealRetrieveOption(option)

        if
            itemID and
            sealID and
            retrievedSealCount
        then
            local availableSeals = player:getSeals(sealID)

            if
                availableSeals >= retrievedSealCount and
                npcUtil.giveItem(player, { { itemID, retrievedSealCount } })
            then
                player:delSeals(retrievedSealCount, sealID)
            end
        end

        -- Purchasing a BCNM Orb
        -- NOTE: While Lua does short-circuit in conditionals, separating the seal check and giveItem calls
        -- to ensure that if this ever changes, it won't cause a potential exploit.
    elseif csid == 322 then
        local itemID, sealType, sealCost = getOrbDataFromOption(option)

        if
            sealType ~= nil and
            player:getSeals(sealType) >= sealCost
        then
            if
                itemID and
                sealCost and
                npcUtil.giveItem(player, itemID)
            then
                player:delSeals(sealCost, sealType)
                player:incrementCharVar('ShamiPurchasedOrbs', 1)
            end
        end
    end
end

return entity
