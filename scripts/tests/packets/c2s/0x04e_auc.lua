local ffi = require('ffi')

if not pcall(ffi.typeof, 'GP_CLI_COMMAND_AUC') then
    ffi.cdef [[
        typedef struct {
            uint16_t id : 9;
            uint16_t size : 7;
            uint16_t sync;
            uint8_t  Command;
            int8_t   AucWorkIndex;
            int8_t   Result;
            int8_t   ResultStatus;
            uint32_t Price;
            uint16_t Item;
            uint16_t padding00;
            uint32_t ItemStacks;
            uint8_t  Parcel[40];
        } GP_CLI_COMMAND_AUC;
    ]]
end

if not pcall(ffi.typeof, 'GP_CLI_COMMAND_PBX') then
    ffi.cdef [[
        typedef struct {
            uint16_t id : 9;
            uint16_t size : 7;
            uint16_t sync;
            uint8_t  Command;
            int8_t   BoxNo;
            int8_t   PostWorkNo;
            int8_t   ItemWorkNo;
            int32_t  ItemStacks;
            int8_t   Result;
            int8_t   ResParam1;
            int8_t   ResParam2;
            int8_t   ResParam3;
            uint8_t  TargetName[16];
        } GP_CLI_COMMAND_PBX;
    ]]
end

local auction =
{
    LOT_IN = 0x0B,
    BID    = 0x0E,
}

local deliveryBox =
{
    RECV      = 0x06,
    POST_OPEN = 0x0E,
    NONE      = -1,
    INCOMING  = 1,
}

local stackPrice = 1000

describe('Auction house', function()
    ---@type CClientEntityPair
    local seller
    ---@type CClientEntityPair
    local buyer

    local function sendAuction(player, command, price, item, single)
        local packet        = ffi.new('GP_CLI_COMMAND_AUC')
        packet.Command      = command
        packet.AucWorkIndex = 0
        packet.Price        = price
        packet.Item         = item
        packet.ItemStacks   = single and 1 or 0

        player.packets:send(0x04E, packet, ffi.sizeof(packet))
    end

    local function sendDeliveryBox(player, command, boxNo, postWorkNo, itemWorkNo)
        local packet      = ffi.new('GP_CLI_COMMAND_PBX')
        packet.Command    = command
        packet.BoxNo      = boxNo
        packet.PostWorkNo = postWorkNo
        packet.ItemWorkNo = itemWorkNo
        packet.ItemStacks = -1

        player.packets:send(0x04D, packet, ffi.sizeof(packet))
    end

    local function u16(data, offset)
        return data[offset] + data[offset + 1] * 256
    end

    local function receiveProceeds(player, slot)
        player.packets:clear()
        sendDeliveryBox(player, deliveryBox.POST_OPEN, deliveryBox.NONE, -1, -1)
        sendDeliveryBox(player, deliveryBox.RECV, deliveryBox.INCOMING, slot, 1)
        xi.test.world:tick()

        for _, incoming in pairs(player.packets:getIncoming()) do
            if
                incoming.type == 0x04B and
                incoming.data[4] == deliveryBox.RECV and
                u16(incoming.data, 48) == xi.item.GIL
            then
                return incoming.data
            end
        end

        assert(false, 'seller did not receive the sale proceeds')
    end

    before_each(function()
        seller = xi.test.world:spawnPlayer({ zone = xi.zone.LOWER_JEUNO })
        buyer  = xi.test.world:spawnPlayer({ zone = xi.zone.LOWER_JEUNO })
        seller:addItem(xi.item.GIL, 1000)
        buyer:addItem(xi.item.GIL, stackPrice)
    end)

    it('records the stack size of a sold stack in the seller delivery box', function()
        local crystals = seller:addItem({ id = xi.item.FIRE_CRYSTAL, quantity = 12 })
        sendAuction(seller, auction.LOT_IN, stackPrice, crystals:getSlotID())
        seller.assert.no:hasItem(xi.item.FIRE_CRYSTAL)

        sendAuction(buyer, auction.BID, stackPrice, xi.item.FIRE_CRYSTAL)
        buyer.assert:hasItem(xi.item.FIRE_CRYSTAL)

        local proceeds = receiveProceeds(seller, 0)
        assert(u16(proceeds, 44) == xi.item.FIRE_CRYSTAL, string.format('proceeds are for item %d', u16(proceeds, 44)))
        assert(proceeds[46] == 12, string.format('proceeds record %d sold, expected the stack of 12', proceeds[46]))
    end)

    it('records one item for a single sale after a stack sale', function()
        local crystals = seller:addItem({ id = xi.item.FIRE_CRYSTAL, quantity = 12 })
        sendAuction(seller, auction.LOT_IN, stackPrice, crystals:getSlotID())
        sendAuction(buyer, auction.BID, stackPrice, xi.item.FIRE_CRYSTAL)
        assert(receiveProceeds(seller, 0)[46] == 12, 'stack sale did not record 12')

        local crystal = seller:addItem({ id = xi.item.ICE_CRYSTAL, quantity = 1 })
        sendAuction(seller, auction.LOT_IN, 100, crystal:getSlotID(), true)
        buyer:addItem(xi.item.GIL, 100)
        sendAuction(buyer, auction.BID, 100, xi.item.ICE_CRYSTAL, true)
        buyer.assert:hasItem(xi.item.ICE_CRYSTAL)

        local proceeds = receiveProceeds(seller, 1)
        assert(u16(proceeds, 44) == xi.item.ICE_CRYSTAL, string.format('proceeds are for item %d', u16(proceeds, 44)))
        assert(proceeds[46] == 1, string.format('proceeds record %d sold, expected 1', proceeds[46]))
    end)
end)
