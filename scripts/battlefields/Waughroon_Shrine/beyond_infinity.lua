-----------------------------------
-- Beyond Infinity
-- Wauhroon Shrine Level Break
-----------------------------------
local waughroonID = zones[xi.zone.WAUGHROON_SHRINE]
-----------------------------------

local content = BattlefieldQuest:new({
    battlefieldId    = xi.battlefield.id.BEYOND_INFINITY_WAUGHROON_SHRINE,

    questArea        = xi.questLog.JEUNO,
    quest            = xi.quest.id.jeuno.BEYOND_INFINITY,

    requiredKeyItems =
    {
        xi.keyItem.SOUL_GEM_CLASP,
        onlyInitiator = true,
        message =
        {
            waughroonID.text.SOUL_GEM_REACTS,
            {
                xi.keyItem.SOUL_GEM_CLASP,
                xi.keyItem.SOUL_GEM,
            }
        },
        deleteMessage = waughroonID.text.LOST_KEYITEM,
    },
})

content.groups =
{
    {
        mobIds =
        {
            { waughroonID.mob.ATORI_TUTORI     },
            { waughroonID.mob.ATORI_TUTORI + 1 },
            { waughroonID.mob.ATORI_TUTORI + 2 },
        },

        allDeath = function(battlefield, mob)
            battlefield:setStatus(xi.battlefield.status.WON)
        end,
    },
}

return content:register()
