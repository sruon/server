-----------------------------------
-- Area: Cloister of Tremors
-- BCNM: Trial by Earth
-----------------------------------

local content = BattlefieldQuest:new({
    battlefieldId    = xi.battlefield.id.TRIAL_BY_EARTH,
    requiredKeyItems = { xi.keyItem.TUNING_FORK_OF_EARTH },

    questArea        = xi.questLog.BASTOK,
    quest            = xi.quest.id.bastok.TRIAL_BY_EARTH,
})

content.groups =
{
    {
        mobs = { 'Titan_Prime_TBE' },
        allDeath = function(battlefield, mob)
            battlefield:setStatus(xi.battlefield.status.WON)
        end,
    },
}

return content:register()
