describe('Retry after hook cleanup', function()
    local originalFunction = GetServerVariable
    local originalSetting  = xi.test.world:getSetting('map.EXP_RETAIN')
    local afterCalls       = 0

    after_each(function()
        afterCalls = afterCalls + 1
        if afterCalls == 1 then
            stub('GetServerVariable', 123)
            xi.test.world:setSetting('map.EXP_RETAIN', originalSetting + 1)
            error('EXPECTED_RETRY_AFTER_FAILURE')
        end
    end)

    it('retries an after hook failure with restored overrides', function()
        assert(GetServerVariable == originalFunction, 'LEAKED_RETRY_AFTER_STUB')
        assert(xi.test.world:getSetting('map.EXP_RETAIN') == originalSetting, 'LEAKED_RETRY_AFTER_SETTING')
    end)

    it('continues after the successful cleanup retry', function()
        assert(afterCalls == 2, 'WRONG_AFTER_RETRY_COUNT')
    end)
end)
