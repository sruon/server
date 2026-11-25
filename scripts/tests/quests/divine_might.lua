describe('Divine Might', function()
    ---@type CClientEntityPair
    local player

    before_each(function()
        player = xi.test.world:spawnPlayer({ zone = xi.zone.THE_SHRINE_OF_RUAVITAU })
        player:addMission(xi.mission.log_id.ZILART, xi.mission.id.zilart.ARK_ANGELS)
        player:setMissionStatus(xi.mission.log_id.ZILART, 1)
        player:addKeyItem(xi.keyItem.SHARD_OF_APATHY)
    end)

    it("should begin the quest with Yve'noile's cutscene", function()
        player.entities:gotoAndTrigger('blank_divine_might', { eventId = 53 })
        player.assert:hasQuest(xi.questLog.OUTLANDS, xi.quest.id.outlands.DIVINE_MIGHT)
    end)

    it('should play the reminder once the quest is accepted', function()
        player.entities:gotoAndTrigger('blank_divine_might', { eventId = 53 })
        player.entities:gotoAndTrigger('blank_divine_might', { eventId = 54 })
        player.assert:hasQuest(xi.questLog.OUTLANDS, xi.quest.id.outlands.DIVINE_MIGHT)
    end)
end)
