local persistent = '[xi_test]fixture'
local volatile   = '[xi_test]volatileFixture'

SetServerVariable('[xi_test]collection', 81)

describe('Server variable fixtures', function()
    local tries = 0

    setup(function()
        SetServerVariable(persistent, 17)
        SetVolatileServerVariable(volatile, 19)
    end)

    describe('nested fixture', function()
        setup(function()
            assert(GetServerVariable(persistent) == 17)
            assert(GetVolatileServerVariable(volatile) == 19)
            SetServerVariable(persistent, 21)
            SetVolatileServerVariable(volatile, 23)
        end)

        before_each(function()
            assert(GetServerVariable(persistent) == 21, 'Persistent fixture leaked')
            assert(GetVolatileServerVariable(volatile) == 23, 'Volatile fixture leaked')
            tries = tries + 1
            if tries == 1 then
                SetServerVariable(persistent, 31)
                SetVolatileServerVariable(volatile, 33)
                error('Injected before_each failure')
            end
        end)

        after_each(function()
            SetServerVariable(persistent, 41)
            SetVolatileServerVariable(volatile, 43)
        end)

        it('restores a failed before hook before retry', function()
            assert(tries == 2)
        end)

        it('restores a failed body before retry', function()
            if tries == 3 then
                SetServerVariable(persistent, 51)
                SetVolatileServerVariable(volatile, 53)
                error('Injected body failure')
            end

            assert(tries == 4)
        end)

        teardown(function()
            assert(GetServerVariable(persistent) == 21)
            assert(GetVolatileServerVariable(volatile) == 23)
            SetServerVariable(persistent, 61)
            SetVolatileServerVariable(volatile, 63)
        end)
    end)

    describe('sibling fixture', function()
        it('inherits the parent setup', function()
            assert(GetServerVariable(persistent) == 17)
            assert(GetVolatileServerVariable(volatile) == 19)
        end)
    end)

    teardown(function()
        SetServerVariable(persistent, 71)
        SetVolatileServerVariable(volatile, 73)
    end)
end)

describe('Failed server variable setup', function()
    setup(function()
        SetServerVariable('[xi_test]badSetup', 91)
        SetVolatileServerVariable('[xi_test]badVolatileSetup', 93)
        error('Injected suite setup failure')
    end)

    it('skips this body', function()
        error('Unexpected body execution')
    end)
end)

describe('Failed server variable teardown', function()
    teardown(function()
        SetServerVariable('[xi_test]badTeardown', 95)
        SetVolatileServerVariable('[xi_test]badVolatileTeardown', 97)
        error('Injected suite teardown failure')
    end)
end)
