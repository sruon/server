local originalFunction = GetServerVariable
local originalSetting  = xi.test.world:getSetting('map.EXP_RETAIN')
local outerCalls       = 0

describe('Failed after hook cleanup', function()
    after_each(function()
        outerCalls = outerCalls + 1
        error('EXPECTED_OUTER_FAILURE')
    end)

    describe('inner', function()
        after_each(function()
            assert(GetServerVariable == originalFunction, 'BODY_STUB_VISIBLE_TO_AFTER')
            assert(xi.test.world:getSetting('map.EXP_RETAIN') == originalSetting, 'BODY_SETTING_VISIBLE_TO_AFTER')
            stub('GetServerVariable', 456)
            xi.test.world:setSetting('map.EXP_RETAIN', originalSetting + 2)
            error('EXPECTED_INNER_FAILURE')
        end)

        it('retains the body error before both cleanup errors', function()
            stub('GetServerVariable', 123)
            xi.test.world:setSetting('map.EXP_RETAIN', originalSetting + 1)
            error('EXPECTED_BODY_FAILURE')
        end)

        it('fails a successful body when cleanup fails', function()
            assert(outerCalls == 1, 'OUTER_HOOK_NOT_RUN')
        end)
    end)
end)

describe('After failure sentinel', function()
    it('restores overrides installed by after hooks', function()
        assert(outerCalls == 2, 'OUTER_HOOK_NOT_RUN')
        assert(GetServerVariable == originalFunction, 'LEAKED_AFTER_STUB')
        assert(xi.test.world:getSetting('map.EXP_RETAIN') == originalSetting, 'LEAKED_AFTER_SETTING')
        assert(xi.settings.map.EXP_RETAIN == originalSetting, 'LEAKED_AFTER_LUA_SETTING')
    end)
end)
