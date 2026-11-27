local originalFunction = GetServerVariable
local originalSetting  = xi.test.world:getSetting('map.EXP_RETAIN')
local originalDebug    = xi.test.world:getSetting('main.DEBUG_CHOCOBO_RAISING')
local settingsTable    = xi.settings.map

xi.test.cleanupProbe =
{
    target = GetServerVariable,
}

describe('Override restoration failure', function()
    after_each(function()
        local remainingRestored = GetServerVariable == originalFunction and
            xi.test.world:getSetting('main.DEBUG_CHOCOBO_RAISING') == originalDebug
        xi.test.cleanupProbe = {}
        xi.settings.map = settingsTable
        assert(remainingRestored, 'REMAINING_CLEANUP_NOT_RUN')
    end)

    it('retains body and restoration errors while finishing cleanup', function()
        stub('xi.test.cleanupProbe.target', 123)
        stub('GetServerVariable', 456)
        xi.test.world:setSetting('map.EXP_RETAIN', originalSetting + 1)
        xi.test.world:setSetting('main.DEBUG_CHOCOBO_RAISING', not originalDebug)
        xi.test.cleanupProbe = nil
        xi.settings.map = nil
        error('EXPECTED_RESTORE_BODY_FAILURE')
    end)
end)

describe('Restoration failure sentinel', function()
    it('retries failed restorations after after hooks repair their targets', function()
        assert(xi.test.cleanupProbe.target == originalFunction, 'FAILED_STUB_NOT_RETRIED')
        assert(xi.settings.map.EXP_RETAIN == originalSetting, 'FAILED_SETTING_NOT_RETRIED')
        assert(GetServerVariable == originalFunction, 'LEAKED_RESTORE_STUB')
        assert(xi.test.world:getSetting('map.EXP_RETAIN') == originalSetting, 'LEAKED_RESTORE_SETTING')
        assert(xi.test.world:getSetting('main.DEBUG_CHOCOBO_RAISING') == originalDebug, 'LEAKED_DEBUG_SETTING')
        xi.test.cleanupProbe = nil
    end)
end)
