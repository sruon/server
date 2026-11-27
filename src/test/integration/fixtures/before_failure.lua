describe('Failed before hook cleanup', function()
    local originalFunction = GetServerVariable
    local originalSetting  = xi.test.world:getSetting('map.EXP_RETAIN')
    local beforeCalls      = 0
    local afterCalls       = 0

    before_each(function()
        beforeCalls = beforeCalls + 1
        if beforeCalls == 1 then
            stub('GetServerVariable', 123)
            xi.test.world:setSetting('map.EXP_RETAIN', originalSetting + 1)
            error('EXPECTED_BEFORE_FAILURE')
        end

        assert(GetServerVariable == originalFunction, 'LEAKED_BEFORE_STUB')
        assert(xi.test.world:getSetting('map.EXP_RETAIN') == originalSetting, 'LEAKED_BEFORE_SETTING')
    end)

    after_each(function()
        afterCalls = afterCalls + 1
        assert(GetServerVariable == originalFunction, 'BEFORE_STUB_VISIBLE_TO_AFTER')
        assert(xi.settings.map.EXP_RETAIN == originalSetting, 'BEFORE_SETTING_VISIBLE_TO_AFTER')
    end)

    it('reports the before failure without running the body', function()
        error('UNEXPECTED_BODY_EXECUTION')
    end)

    it('starts the next case with restored overrides', function()
        assert(afterCalls == 1, 'AFTER_HOOK_NOT_RUN')
    end)
end)
