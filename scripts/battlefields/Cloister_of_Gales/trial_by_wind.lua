-----------------------------------
-- Area: Cloister of Gales
-- BCNM: Trial by Wind
-----------------------------------

local content = BattlefieldQuest:new({
    battlefieldId    = xi.battlefield.id.TRIAL_BY_WIND,
    requiredKeyItems = { xi.keyItem.TUNING_FORK_OF_WIND },

    questArea        = xi.questLog.OUTLANDS,
    quest            = xi.quest.id.outlands.TRIAL_BY_WIND,
})

function content:onEventFinishWin(player, csid, option, npc)
    player:addTitle(xi.title.HEIR_OF_THE_GREAT_WIND)
    npcUtil.giveKeyItem(player, xi.keyItem.WHISPER_OF_GALES)
end

content.groups =
{
    {
        mobs = { 'Garuda_Prime_TBW' },
        allDeath = function(battlefield, mob)
            battlefield:setStatus(xi.battlefield.status.WON)
        end,
    },
}

return content:register()
