-----------------------------------
-- Area: Cloister of Storms
-- BCNM: Trial by Lightning
-----------------------------------

local content = BattlefieldQuest:new({
    battlefieldId    = xi.battlefield.id.TRIAL_BY_LIGHTNING,
    requiredKeyItems = { xi.keyItem.TUNING_FORK_OF_LIGHTNING },

    questArea        = xi.questLog.OTHER_AREAS,
    quest            = xi.quest.id.otherAreas.TRIAL_BY_LIGHTNING,
})

function content:onEventFinishWin(player, csid, option, npc)
    player:addTitle(xi.title.HEIR_OF_THE_GREAT_LIGHTNING)
    npcUtil.giveKeyItem(player, xi.keyItem.WHISPER_OF_STORMS)
end

content.groups =
{
    {
        mobs = { 'Ramuh_Prime_TBL' },
        allDeath = function(battlefield, mob)
            battlefield:setStatus(xi.battlefield.status.WON)
        end,
    },
}

return content:register()
