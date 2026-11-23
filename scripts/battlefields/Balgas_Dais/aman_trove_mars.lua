-----------------------------------
-- A.M.A.N. Trove (Mars Orb)
-- Balga's Dais
-- !additem 9275
-----------------------------------
local balgasID = zones[xi.zone.BALGAS_DAIS]
-----------------------------------

local content = Battlefield:new({
    battlefieldId = xi.battlefield.id.AMAN_TROVE_MARS_BALGAS_DAIS,
    requiredItems = { xi.item.MARS_ORB, wearMessage = balgasID.text.A_CRACK_HAS_FORMED, wornMessage = balgasID.text.ORB_IS_CRACKED },
})

function content:entryRequirement(player, npc, isRegistrant, trade)
    return xi.settings.main.ENABLE_AMAN_TROVE == 1
end

function content:setupBattlefield(battlefield)
    xi.amanTrove.setupBattlefield(battlefield)
end

content.groups =
{
    {
        mobs = { 'Chest_OPlenty' },
    },
}

return content:register()
