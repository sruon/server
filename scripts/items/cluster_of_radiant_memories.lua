-----------------------------------
-- ID: 5292
-- Cluster of radiant memories
-- Turn into a stack of radiant memories
-----------------------------------
---@type TItem
local itemObject = {}

itemObject.onItemCheck = function(target, item, caster)
    return xi.itemUtils.itemBoxOnItemCheck(target)
end

itemObject.onItemUse = function(target)
    npcUtil.giveItem(target, { { xi.item.REMNANT_OF_A_RADIANT_MEMORY, 12 } })
end

return itemObject
