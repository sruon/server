-----------------------------------
-- Area: Empyreal_Paradox
-- Name: Apocalypse Nigh
-----------------------------------
local empyrealParadoxID = zones[xi.zone.EMPYREAL_PARADOX]
-----------------------------------

local content = BattlefieldQuest:new({
    battlefieldId = xi.battlefield.id.APOCALYPSE_NIGH,
    questArea     = xi.questLog.JEUNO,
    quest         = xi.quest.id.jeuno.APOCALYPSE_NIGH,
    requiredVar   = 'Quest[3][89]Prog',
    requiredValue = 3,
})

function content:onEventFinishWin(player, csid, option, npc)
    player:setPos(540, 0, -514, 63, xi.zone.EMPYREAL_PARADOX)
end

content.groups =
{
    {
        mobIds =
        {
            {
                empyrealParadoxID.mob.KAMLANAUT,
                empyrealParadoxID.mob.KAMLANAUT + 1,
            },

            {
                empyrealParadoxID.mob.KAMLANAUT + 2,
                empyrealParadoxID.mob.KAMLANAUT + 3,
            },

            {
                empyrealParadoxID.mob.KAMLANAUT + 4,
                empyrealParadoxID.mob.KAMLANAUT + 5,
            },
        },

        superlink = true,
        allDeath = function(battlefield, mob)
            battlefield:setStatus(xi.battlefield.status.WON)
        end,
    },
}

return content:register()
