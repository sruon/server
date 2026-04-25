-----------------------------------
-- Bite the Dust
-----------------------------------
-- Log ID: 1, Quest ID: 46
-- Yazan : !pos -20.06 -3.3 24.471 236
-----------------------------------

local quest = Quest:new(xi.questLog.BASTOK, xi.quest.id.bastok.BITE_THE_DUST)

quest.reward =
{
    fame     = 8,
    fameArea = xi.fameArea.BASTOK,
    gil      = 350,
    title    = xi.title.SAND_BLASTER,
}

quest.sections =
{
    {
        check = function(player, status, vars)
            return status == xi.questStatus.QUEST_AVAILABLE and
                player:getFameLevel(xi.fameArea.BASTOK) >= 2
        end,

        [xi.zone.PORT_BASTOK] =
        {
            ['Yazan'] = quest:progressEvent(191),

            onEventFinish =
            {
                [191] = function(player, csid, option, npc)
                    quest:begin(player)
                end,
            },
        },
    },

    {
        check = function(player, status, vars)
            return status ~= xi.questStatus.QUEST_AVAILABLE
        end,

        [xi.zone.PORT_BASTOK] =
        {
            ['Yazan'] =
            {
                declaredTrades =
                {
                    {
                        match = { items = { { xi.item.SAND_BAT_FANG, 1 } } },
                        event =
                        {
                            id       = 193,
                            onFinish = function(player, option, npc)
                                if player:getQuestStatus(quest.areaId, quest.questId) == xi.questStatus.QUEST_ACCEPTED then
                                    player:addFame(xi.fameArea.BASTOK, 112)
                                end

                                quest:complete(player)
                                return true
                            end,
                        },
                    },
                },

                onTrigger = function(player, npc)
                    local questStatus = player:getQuestStatus(quest.areaId, quest.questId)

                    if questStatus == xi.questStatus.QUEST_ACCEPTED then
                        return quest:event(192)
                    else
                        return quest:event(194):oncePerZone()
                    end
                end,
            },
        },
    },
}

return quest
