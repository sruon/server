describe('Retry cleanup', function()
    local originalFunction = GetServerVariable
    local originalSetting  = xi.test.world:getSetting('map.EXP_RETAIN')
    local beforeCalls      = 0
    local afterCalls       = 0
    local setupCalls       = 0

    setup(function()
        setupCalls = setupCalls + 1
    end)

    before_each(function()
        beforeCalls = beforeCalls + 1
        assert(GetServerVariable == originalFunction, 'LEAKED_RETRY_STUB')
        assert(xi.test.world:getSetting('map.EXP_RETAIN') == originalSetting, 'LEAKED_RETRY_SETTING')
        if beforeCalls == 1 then
            stub('GetServerVariable', 123)
            xi.test.world:setSetting('map.EXP_RETAIN', originalSetting + 1)
            error('EXPECTED_RETRY_FAILURE')
        end
    end)

    after_each(function()
        afterCalls = afterCalls + 1
    end)

    it('retries with restored overrides and shared setup', function()
        assert(beforeCalls == 2, 'WRONG_ATTEMPT_COUNT')
        assert(afterCalls == 1, 'AFTER_HOOK_NOT_RUN')
        assert(setupCalls == 1, 'SETUP_REPEATED')
    end)

    it('keeps the next case clean after a retry', function()
        assert(afterCalls == 2, 'WRONG_AFTER_COUNT')
        assert(setupCalls == 1, 'SETUP_REPEATED')
    end)
end)
