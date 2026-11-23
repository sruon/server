-----------------------------------
-- Beyond Infinity
-- Qu'Bia Arena Level Break
-----------------------------------
local qubiaID = zones[xi.zone.QUBIA_ARENA]
-----------------------------------

local content = BattlefieldQuest:new({
    battlefieldId    = xi.battlefield.id.BEYOND_INFINITY,

    questArea        = xi.questLog.JEUNO,
    quest            = xi.quest.id.jeuno.BEYOND_INFINITY,

    requiredKeyItems =
    {
        xi.keyItem.SOUL_GEM_CLASP,
        onlyInitiator = true,
        message =
        {
            qubiaID.text.SOUL_GEM_REACTS,
            {
                xi.keyItem.SOUL_GEM_CLASP,
                xi.keyItem.SOUL_GEM,
            }
        },
        deleteMessage = qubiaID.text.LOST_KEYITEM,
    },
})

content.groups =
{
    {
        mobIds =
        {
            { qubiaID.mob.ATORI_TUTORI     },
            { qubiaID.mob.ATORI_TUTORI + 1 },
            { qubiaID.mob.ATORI_TUTORI + 2 },
        },

        allDeath = function(battlefield, mob)
            battlefield:setStatus(xi.battlefield.status.WON)
        end,
    },
}

return content:register()
