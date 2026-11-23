-----------------------------------
-- Area: Cloister of Flames
-- BCNM: Trial by Fire
-----------------------------------

local content = BattlefieldQuest:new({
    battlefieldId    = xi.battlefield.id.TRIAL_BY_FIRE,
    requiredKeyItems = { xi.keyItem.TUNING_FORK_OF_FIRE },

    questArea        = xi.questLog.OUTLANDS,
    quest            = xi.quest.id.outlands.TRIAL_BY_FIRE,
})

content.groups =
{
    {
        mobs = { 'Ifrit_Prime_TBF' },
        allDeath = function(battlefield, mob)
            battlefield:setStatus(xi.battlefield.status.WON)
        end,
    },
}

return content:register()
