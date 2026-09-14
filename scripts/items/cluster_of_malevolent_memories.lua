-----------------------------------
-- ID: 5293
-- Cluster of malevolent memories
-- Turn into a stack of malevolent memories
-----------------------------------
---@type TItem
local itemObject = {}

itemObject.onItemCheck = function(target, item, caster)
    return xi.itemUtils.itemBoxOnItemCheck(target)
end

itemObject.onItemUse = function(target)
    npcUtil.giveItem(target, { { xi.item.REMNANT_OF_A_MALEVOLENT_MEMORY, 12 } })
end

return itemObject
