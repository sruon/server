local collection = GetServerVariable('[xi_test]collection')
local besieged   = GetServerVariable('[Besieged]ChochoroonCaptured')

describe('Server variables across files', function()
    it('restores collection before loading this file', function()
        assert(collection == 0)
    end)

    it('restores persistent suite writes', function()
        assert(GetServerVariable('[xi_test]fixture') == 0)
    end)

    it('restores volatile suite writes', function()
        assert(GetVolatileServerVariable('[xi_test]volatileFixture') == 0)
    end)

    it('restores failed suite setup', function()
        assert(GetServerVariable('[xi_test]badSetup') == 0)
        assert(GetVolatileServerVariable('[xi_test]badVolatileSetup') == 0)
    end)

    it('restores failed suite teardown', function()
        assert(GetServerVariable('[xi_test]badTeardown') == 0)
        assert(GetVolatileServerVariable('[xi_test]badVolatileTeardown') == 0)
    end)

    it('restores the captured Besieged prisoner', function()
        assert(GetServerVariable('[Besieged]ChochoroonCaptured') == besieged)
    end)
end)
