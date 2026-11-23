-----------------------------------
-- Beyond Infinity
-- Horlais Peak Level Break
-----------------------------------
local horlaisID = zones[xi.zone.HORLAIS_PEAK]
-----------------------------------

local content = BattlefieldQuest:new({
    battlefieldId    = xi.battlefield.id.BEYOND_INFINITY_HORLAIS_PEAK,

    questArea        = xi.questLog.JEUNO,
    quest            = xi.quest.id.jeuno.BEYOND_INFINITY,

    requiredKeyItems =
    {
        xi.keyItem.SOUL_GEM_CLASP,
        onlyInitiator = true,
        message =
        {
            horlaisID.text.SOUL_GEM_REACTS,
            {
                xi.keyItem.SOUL_GEM_CLASP,
                xi.keyItem.SOUL_GEM,
            }
        },
        deleteMessage = horlaisID.text.LOST_KEYITEM,
    },
})

content.groups =
{
    {
        mobIds =
        {
            { horlaisID.mob.ATORI_TUTORI     },
            { horlaisID.mob.ATORI_TUTORI + 1 },
            { horlaisID.mob.ATORI_TUTORI + 2 },
        },

        allDeath = function(battlefield, mob)
            battlefield:setStatus(xi.battlefield.status.WON)
        end,
    },
}

return content:register()
