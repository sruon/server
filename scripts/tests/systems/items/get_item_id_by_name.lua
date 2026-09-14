describe('GetItemIDByName', function()
    it('finds an item by its name', function()
        assert(GetItemIDByName('potion') == xi.item.POTION, 'potion was not found')
    end)

    it('ignores case', function()
        assert(GetItemIDByName('POTION') == xi.item.POTION, 'POTION was not found')
        assert(GetItemIDByName('Potion') == xi.item.POTION, 'Potion was not found')
    end)

    it('finds an item by its sort name', function()
        assert(GetItemIDByName('chocobo_bedding') == xi.item.PILE_OF_CHOCOBO_BEDDING, 'chocobo_bedding was not found')
    end)

    it('only matches whole names', function()
        assert(GetItemIDByName('potio') == 0, 'potio matched an item')
        assert(GetItemIDByName('potion%') == 0, 'potion% matched an item')
    end)

    it('returns 0 for an unknown name', function()
        assert(GetItemIDByName('not_an_item') == 0, 'not_an_item matched an item')
    end)
end)
