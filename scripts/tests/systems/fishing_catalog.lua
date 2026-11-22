describe('Fishing catalog', function()
    ---@type CClientEntityPair
    local player

    before_each(function()
        player = xi.test.world:spawnPlayer({ zone = xi.zone.SOUTHERN_SAN_DORIA })
    end)

    -- Only catalog entries in play get fish exdata
    local function hasFishExdata(itemId)
        local item = player:addItem({ id = itemId, quantity = 1 })
        assert(item, string.format('could not add item %d', itemId))

        return item:getExData().size ~= nil
    end

    it('gives fish exdata to fish and items in the catalog', function()
        assert(hasFishExdata(xi.item.BLACK_SOLE), 'Black Sole has no fish exdata')
        assert(hasFishExdata(xi.item.ARROWWOOD_LOG), 'Arrowwood Log has no fish exdata')
    end)

    it('leaves out disabled and ranking 99 fish', function()
        assert(not hasFishExdata(xi.item.ABAIA), 'disabled Abaia has fish exdata')
        assert(not hasFishExdata(xi.item.GARPIKE), 'ranking 99 Garpike has fish exdata')
    end)

    it('leaves out items that are not in the catalog', function()
        assert(not hasFishExdata(xi.item.CREEL_OF_MOAT_CARP), 'Creel of Moat Carp has fish exdata')
    end)
end)
