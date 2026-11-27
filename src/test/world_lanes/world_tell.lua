local ffi = require('ffi')
local tellType = ffi.typeof([[
    struct {
        uint16_t header;
        uint16_t sequence;
        uint8_t kind;
        uint8_t padding;
        char recipient[15];
        char message[128];
    }
]])

describe('Shared world router', function()
    it('returns the offline tell response to its sender', function()
        local player = xi.test.world:spawnPlayer()
        player.packets:clear()
        local tell = tellType()
        tell.kind = 3
        ffi.copy(tell.recipient, 'Absentworld')
        ffi.copy(tell.message, 'test world route')
        player.packets:send(0x0B6, tell, ffi.sizeof(tell))

        local responses = 0
        for _, packet in ipairs(player.packets:getIncoming()) do
            if packet.type == 0x009 then
                local data = packet.data
                local recipient = data[4] + data[5] * 256 + data[6] * 65536 + data[7] * 16777216
                local message = data[10] + data[11] * 256
                assert(recipient == player:getID(), 'World reply addressed a different character')
                assert(message == 125, 'Expected the offline tell response')
                responses = responses + 1
            end
        end
        assert(responses == 1, 'Expected exactly one world reply')
    end)
end)
