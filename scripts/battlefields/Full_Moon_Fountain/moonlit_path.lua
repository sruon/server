-----------------------------------
-- Area: Full Moon Fountain
-- Name: The Moonlit Path
-----------------------------------
local fullMoonFountainID = zones[xi.zone.FULL_MOON_FOUNTAIN]
-----------------------------------

local content = BattlefieldQuest:new({
    battlefieldId    = xi.battlefield.id.MOONLIT_PATH,
    requiredKeyItems = { xi.keyItem.MOON_BAUBLE, keep = true },

    questArea        = xi.questLog.WINDURST,
    quest            = xi.quest.id.windurst.THE_MOONLIT_PATH,
})

content.groups =
{
    {
        mobIds =
        {
            { fullMoonFountainID.mob.FENRIR_PRIME     },
            { fullMoonFountainID.mob.FENRIR_PRIME + 1 },
            { fullMoonFountainID.mob.FENRIR_PRIME + 2 },
        },

        allDeath = function(battlefield, mob)
            battlefield:setStatus(xi.battlefield.status.WON)
        end,
    },
}

return content:register()
