-----------------------------------
-- Area: Cloister of Frost
-- BCNM: Trial by Ice
-----------------------------------

local content = BattlefieldQuest:new({
    battlefieldId    = xi.battlefield.id.TRIAL_BY_ICE,
    requiredKeyItems = { xi.keyItem.TUNING_FORK_OF_ICE },

    questArea        = xi.questLog.SANDORIA,
    quest            = xi.quest.id.sandoria.TRIAL_BY_ICE,
})

function content:onEventFinishWin(player, csid, option, npc)
    player:addTitle(xi.title.HEIR_OF_THE_GREAT_ICE)
    npcUtil.giveKeyItem(player, xi.keyItem.WHISPER_OF_FROST)
end

content.groups =
{
    {
        mobs = { 'Shiva_Prime_TBI' },
        allDeath = function(battlefield, mob)
            battlefield:setStatus(xi.battlefield.status.WON)
        end,
    },
}

return content:register()
