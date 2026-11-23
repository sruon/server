-----------------------------------
-- A.M.A.N. Trove (Venus Orb)
-- Balga's Dais
-- !additem 9276
-----------------------------------
local balgasID = zones[xi.zone.BALGAS_DAIS]
-----------------------------------

local content = Battlefield:new({
    battlefieldId = xi.battlefield.id.AMAN_TROVE_VENUS_BALGAS_DAIS,
    requiredItems = { xi.item.VENUS_ORB, wearMessage = balgasID.text.A_CRACK_HAS_FORMED, wornMessage = balgasID.text.ORB_IS_CRACKED },
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
        mobs = { 'Coffer_OPlenty' },
    },
}

return content:register()
