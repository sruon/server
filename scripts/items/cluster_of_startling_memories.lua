-----------------------------------
-- ID: 5290
-- Cluster of startling memories
-- Turn into a stack of startling memories
-----------------------------------
---@type TItem
local itemObject = {}

itemObject.onItemCheck = function(target, item, caster)
    return xi.itemUtils.itemBoxOnItemCheck(target)
end

itemObject.onItemUse = function(target)
    npcUtil.giveItem(target, { { xi.item.REMNANT_OF_A_STARTLING_MEMORY, 12 } })
end

return itemObject
