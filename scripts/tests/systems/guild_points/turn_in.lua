describe('Guild point turn-ins', function()
    ---@type CClientEntityPair
    local player

    before_each(function()
        player = xi.test.world:spawnPlayer({ zone = xi.zone.PORT_WINDURST })
        player:setCharVar('[GUILD]currentGuild', xi.guild.FISHING + 1)
        player:setCharVar('[GUILD]daily_points', 0)
    end)

    it('requests an item with points left to earn', function()
        local gpItem, remainingPoints = player:getCurrentGPItem(xi.guild.FISHING)

        assert(gpItem ~= 0, 'no guild point item requested')
        assert(remainingPoints > 0, 'no points left to earn')
    end)

    it('awards the item points as guild currency', function()
        local gpItem, remainingBefore = player:getCurrentGPItem(xi.guild.FISHING)
        local currencyBefore          = player:getCurrency('guild_fishing')

        player:addItem(gpItem)
        player.actions:tradeNpc('Fennella', { gpItem }, { eventId = 10021 })

        local _, remainingAfter = player:getCurrentGPItem(xi.guild.FISHING)
        local earned            = player:getCurrency('guild_fishing') - currencyBefore

        assert(earned > 0, 'no guild points awarded')
        assert(remainingBefore - remainingAfter == earned, 'daily remaining points did not drop by the points earned')
        assert(not player:hasItem(gpItem), 'item not taken')
    end)
end)
