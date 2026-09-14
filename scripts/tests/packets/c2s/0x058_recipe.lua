local ffi = require('ffi')

if not pcall(ffi.typeof, 'GP_CLI_COMMAND_RECIPE') then
    ffi.cdef [[
        typedef struct {
            uint16_t id : 9;
            uint16_t size : 7;
            uint16_t sync;
            uint16_t skill;
            uint16_t level;
            uint16_t Param0;
            uint16_t Mode;
            uint16_t Param1;
            uint16_t Param2;
            uint16_t Param3;
            uint16_t Param4;
        } GP_CLI_COMMAND_RECIPE;
    ]]
end

local mode =
{
    RECIPE_LIST      = 2,
    RECIPE_MATERIALS = 3,
}

local woodworking = 1

describe('Recipe list', function()
    ---@type CClientEntityPair
    local player

    local function u16(data, offset)
        return data[offset] + data[offset + 1] * 256
    end

    local function request(requestMode, param1, param3)
        local packet  = ffi.new('GP_CLI_COMMAND_RECIPE')
        packet.skill  = woodworking
        packet.level  = 10
        packet.Mode   = requestMode
        packet.Param1 = param1
        packet.Param3 = param3
        packet.Param4 = 0

        player.packets:clear()
        player.packets:send(0x058, packet, ffi.sizeof(packet))
        for _, incoming in pairs(player.packets:getIncoming()) do
            if incoming.type == 0x031 then
                return incoming.data
            end
        end

        assert(false, 'no recipe packet received')
    end

    before_each(function()
        player = xi.test.world:spawnPlayer()
    end)

    it('pages in the same order the recipe details are indexed', function()
        local firstPage = request(mode.RECIPE_LIST, 0, 0)
        assert(u16(firstPage, 48) == mode.RECIPE_LIST, 'first page is not a recipe list')

        local listed = {}
        for index = 0, 15 do
            listed[index] = u16(firstPage, 16 + index * 2)
            assert(listed[index] ~= 0, string.format('recipe %d missing from the first page', index))
        end

        listed[16] = u16(firstPage, 50)
        assert(listed[16] ~= 0, 'first page does not point at a second page')

        for index = 0, 16 do
            local details = request(mode.RECIPE_MATERIALS, 0, index)
            assert(u16(details, 48) == mode.RECIPE_MATERIALS, 'details packet has the wrong type')
            assert(u16(details, 4) == listed[index], string.format('recipe %d details are for %d, list shows %d', index, u16(details, 4), listed[index]))
        end

        local secondPage = request(mode.RECIPE_LIST, 16, 0)
        assert(u16(secondPage, 16) == listed[16], 'second page does not start where the first page pointed')
    end)

    it('sends empty details past the last recipe', function()
        local details = request(mode.RECIPE_MATERIALS, 0, 1000)
        assert(u16(details, 4) == 0, 'details sent for a recipe past the end of the list')
    end)
end)
