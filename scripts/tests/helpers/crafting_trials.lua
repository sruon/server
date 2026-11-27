local function grant(player, itemId, quantity)
    local before = player:getItemCount(itemId)
    assert(player:addItem(itemId, quantity) and player:getItemCount(itemId) == before + quantity,
        'incomplete crafting material grant for ' .. itemId)
end

---@param player CClientEntityPair
---@param crystal integer
---@param ingredients integer[]
---@param attempts integer
---@return fun(): integer?
local function craftingTrials(player, crystal, ingredients, attempts)
    local stackSize = 12
    local batchSize = stackSize
    local counts    = {}
    for _, itemId in ipairs(ingredients) do
        counts[itemId] = (counts[itemId] or 0) + 1
        batchSize = math.min(batchSize, math.floor(stackSize / counts[itemId]))
    end

    local trial    = 0
    local batchEnd = 0
    return function()
        if trial > 0 then
            assert(player:getItemCount(crystal) == batchEnd - trial, 'crafting crystal was not consumed')
        end

        if trial == batchEnd then
            if trial > 0 then
                player:delContainerItems(xi.inv.INVENTORY)
                assert(player:getFreeSlotsCount() == player:getContainerSize(xi.inv.INVENTORY), 'crafting inventory did not empty')
            end

            if trial == attempts then
                return nil
            end

            local size = math.min(batchSize, attempts - trial)
            grant(player, crystal, size)
            for itemId, count in pairs(counts) do
                grant(player, itemId, count * size)
            end

            assert(player:getFreeSlotsCount() >= size, 'insufficient crafting result slots')
            batchEnd = trial + size
        end

        trial = trial + 1
        return trial
    end
end

return craftingTrials
