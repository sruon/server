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

local woodworking  = 1
local banquetTable = 3653

describe('Recipe menu', function()
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

    it('lists amateur woodworking in level then item name order', function()
        local expected =
        {
            banquetTable,
            xi.item.FLUTE,
            xi.item.PIECE_OF_ARROWWOOD_LUMBER,
            xi.item.PIECE_OF_ARROWWOOD_LUMBER,
            xi.item.PIECE_OF_LAUAN_LUMBER,
            xi.item.PIECE_OF_LAUAN_LUMBER,
            xi.item.STONE_ARROW,
            xi.item.WOODEN_ARROW,
            xi.item.PADDED_BOX,
            xi.item.PADDED_BOX,
            xi.item.PIECE_OF_MAPLE_LUMBER,
            xi.item.PIECE_OF_MAPLE_LUMBER,
            xi.item.MAPLE_WAND,
            xi.item.LAUAN_SHIELD,
            xi.item.WORKBENCH,
            xi.item.ASH_POLE,
        }

        local page = request(mode.RECIPE_LIST, 0, 0)
        assert(u16(page, 48) == mode.RECIPE_LIST, 'page is not a recipe list')

        for index, itemId in ipairs(expected) do
            local listed = u16(page, 16 + (index - 1) * 2)
            assert(listed == itemId, string.format('entry %d lists %d, expected %d', index - 1, listed, itemId))
        end

        assert(u16(page, 50) == xi.item.PIECE_OF_ASH_LUMBER, 'page does not point at the next one')
    end)

    it('sends the key item, crystal and ingredients of a recipe', function()
        local details = request(mode.RECIPE_MATERIALS, 0, 3)
        assert(u16(details, 48) == mode.RECIPE_MATERIALS, 'details packet has the wrong type')
        assert(u16(details, 4) == xi.item.PIECE_OF_ARROWWOOD_LUMBER, 'wrong result')
        assert(u16(details, 12) == xi.item.WIND_CRYSTAL, 'wrong crystal')
        assert(u16(details, 14) == xi.keyItem.LUMBERJACK, 'wrong key item')
        assert(u16(details, 16) == xi.item.ARROWWOOD_LOG and u16(details, 32) == 3, 'wrong first ingredient')
        assert(u16(details, 18) == xi.item.SPOOL_OF_BUNDLING_TWINE and u16(details, 34) == 1, 'wrong second ingredient')
        assert(u16(details, 20) == 0, 'extra ingredient')
    end)

    it('sends the subcrafts of a recipe', function()
        local details = request(mode.RECIPE_MATERIALS, 0, 13)
        assert(u16(details, 4) == xi.item.LAUAN_SHIELD, 'wrong result')
        assert(u16(details, 6) == 2, 'smithing is not the first subcraft')
        assert(u16(details, 8) == 0, 'extra subcraft')
        assert(u16(details, 12) == xi.item.EARTH_CRYSTAL, 'wrong crystal')
        assert(u16(details, 16) == xi.item.BRONZE_SHEET and u16(details, 32) == 1, 'wrong first ingredient')
        assert(u16(details, 18) == xi.item.PIECE_OF_LAUAN_LUMBER and u16(details, 34) == 2, 'wrong second ingredient')
    end)

    it('pages in the same order the recipe details are indexed', function()
        local firstPage = request(mode.RECIPE_LIST, 0, 0)

        local listed = {}
        for index = 0, 15 do
            listed[index] = u16(firstPage, 16 + index * 2)
        end

        listed[16] = u16(firstPage, 50)

        for index = 0, 16 do
            local details = request(mode.RECIPE_MATERIALS, 0, index)
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
