local originalFunction = GetServerVariable
local originalSetting  = xi.test.world:getSetting('map.EXP_RETAIN')

describe('Successful after hook cleanup', function()
    after_each(function()
        stub('GetServerVariable', 123)
        xi.test.world:setSetting('map.EXP_RETAIN', originalSetting + 1)
    end)

    it('allows successful after hooks to install overrides', function()
        assert(GetServerVariable == originalFunction, 'DIRTY_INITIAL_STATE')
    end)
end)

describe('Successful after hook sentinel', function()
    it('restores overrides after a successful after hook', function()
        assert(GetServerVariable == originalFunction, 'LEAKED_SUCCESSFUL_AFTER_STUB')
        assert(xi.test.world:getSetting('map.EXP_RETAIN') == originalSetting, 'LEAKED_SUCCESSFUL_AFTER_SETTING')
        assert(xi.settings.map.EXP_RETAIN == originalSetting, 'LEAKED_SUCCESSFUL_AFTER_LUA_SETTING')
    end)
end)
