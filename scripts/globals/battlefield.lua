-----------------------------------
-- Battlefield Functions
-----------------------------------
require('scripts/globals/pathfind')
-----------------------------------
xi = xi or {}

xi.loot = xi.loot or {}

xi.loot.weight =
{
    EXTREMELY_LOW  = 2,
    VERY_LOW       = 10,
    LOW            = 30,
    NORMAL         = 50,
    HIGH           = 70,
    VERY_HIGH      = 100,
    EXTREMELY_HIGH = 140,
}

xi.battlefield = xi.battlefield or {}
xi.battlefield.contents = xi.battlefield.contents or {}
xi.battlefield.contentsByZone = xi.battlefield.contentsByZone or {}

xi.battlefield.status =
{
    OPEN     = 0,
    LOCKED   = 1,
    WON      = 2,
    LOST     = 3,
}

xi.battlefield.returnCode =
{
    WAIT              = 1,
    CUTSCENE          = 2,
    INCREMENT_REQUEST = 3,
    LOCKED            = 4,
    REQS_NOT_MET      = 5,
    BATTLEFIELD_FULL  = 6,
}

xi.battlefield.leaveCode =
{
    EXIT   = 1,
    WON    = 2,
    WARPDC = 3,
    LOST   = 4
}

xi.battlefield.itemUses =
{
    [xi.item.WARRIORS_TESTIMONY]      = 3,
    [xi.item.MONKS_TESTIMONY]         = 3,
    [xi.item.WHITE_MAGES_TESTIMONY]   = 3,
    [xi.item.BLACK_MAGES_TESTIMONY]   = 3,
    [xi.item.RED_MAGES_TESTIMONY]     = 3,
    [xi.item.THIEFS_TESTIMONY]        = 3,
    [xi.item.PALADINS_TESTIMONY]      = 3,
    [xi.item.DARK_KNIGHTS_TESTIMONY]  = 3,
    [xi.item.BEASTMASTERS_TESTIMONY]  = 3,
    [xi.item.BARDS_TESTIMONY]         = 3,
    [xi.item.RANGERS_TESTIMONY]       = 3,
    [xi.item.SAMURAIS_TESTIMONY]      = 3,
    [xi.item.NINJAS_TESTIMONY]        = 3,
    [xi.item.DRAGOONS_TESTIMONY]      = 3,
    [xi.item.SUMMONERS_TESTIMONY]     = 3,
    [xi.item.BLUE_MAGES_TESTIMONY]    = 3,
    [xi.item.CORSAIRS_TESTIMONY]      = 3,
    [xi.item.PUPPETMASTERS_TESTIMONY] = 3,
}

Battlefield         = setmetatable({}, { __index = Container })
Battlefield.__index = Battlefield

---@diagnostic disable-next-line: duplicate-set-field
Battlefield.__eq    = function(m1, m2)
    return m1.id == m2.id
end

-- Creates a new Battlefield interaction
-- Data takes the following keys:
--  - battlefieldId: Battlefield ID used in the database (required)
--  - requiredItems: Items required to be traded to enter the battlefield.
--                   Needs to be in the format of { itemid, quantity, useMessage = ID.text.*, wearMessage = ID.text.*, wornMessage = ID.text.* }. (optional)
--  - requiredKeyItems: Key items required to be able to enter the battlefield - these are removed upon entry unless 'keep = true'.
--                      Set 'onlyInitiator' to true to only require the initiator of the battlefield to have the key item. (optional)
--  - title: Title given to players upon victory (optional)
--  - lossEventParams: Parameters given to the loss event (32002). Defaults to none. (optional)
---@diagnostic disable-next-line: duplicate-set-field
function Battlefield:new(data)
    local obj        = Container:new(Battlefield.getVarPrefix(data.battlefieldId))
    local definition = GetBattlefieldDefinition(data.battlefieldId)

    if not definition then
        error(string.format('battlefield %d is not listed in its zone battlefields.yaml', data.battlefieldId))
    end

    setmetatable(obj, self)
    obj.zoneId        = bit.rshift(data.battlefieldId, 5)
    obj.battlefieldId = data.battlefieldId
    obj.index         = bit.band(data.battlefieldId, 31)
    obj.maxAreas      = definition.maxAreas
    obj.maxPlayers    = definition.maxPlayers
    obj.timeLimit     = definition.timeLimit
    obj.entryNpcs     = definition.entryNpcs
    obj.exitNpcs      = definition.exitNpcs
    obj.area          = definition.area
    obj.allowedAreas  = definition.allowedAreas and set(definition.allowedAreas)

    obj.title            = data.title
    obj.grantXP          = definition.grantXP
    obj.grantXPLockout   = definition.grantXPLockout
    obj.allowTrusts      = definition.allowTrusts
    obj.rhapsodyTrusts   = definition.rhapsodyTrusts
    obj.hasWipeGrace     = definition.hasWipeGrace
    obj.cleanupDebuffs   = definition.cleanupDebuffs
    obj.delayToExit      = definition.delayToExit
    obj.requiredItems    = data.requiredItems or {}
    obj.requiredKeyItems = data.requiredKeyItems or {}
    obj.lossEventParams  = data.lossEventParams or {}
    obj.armouryCrates    = data.armouryCrates or false
    obj.experimental     = definition.experimental
    obj.csParam7         = data.csParam7 and data.csParam7 or 0
    obj.csParam8         = data.csParam8 and data.csParam8 or 0

    obj.sections = obj.sections or { { [obj.zoneId] = {} } }
    obj.groups   = {}
    obj.paths    = {}
    obj.loot     = {}

    -- Determine which items need to be traded as the requiredItems format is incompatable with npcUtil.tradeHasExactly
    obj.tradeItems = {}

    for index, value in ipairs(obj.requiredItems) do
        obj.tradeItems[index] = value
    end

    return obj
end

function Battlefield.getVarPrefix(battlefieldID)
    return string.format('Battlefield[%d]', battlefieldID)
end

function Battlefield:register()
    -- Only hookup the entry and exit listeners if there aren't any other battlefields already registered for that entrance
    local setupEvents    = true
    local setupEntryNpcs = true

    if utils.hasKey(self.zoneId, xi.battlefield.contentsByZone) then
        local contents = xi.battlefield.contentsByZone[self.zoneId]

        for _, content in ipairs(contents) do
            -- Always setup listeners if we're reloading a battlefield
            if self.battlefieldId == content.battlefieldId and content.hasListeners then
                setupEvents    = true
                setupEntryNpcs = true

                break
            end

            -- Do not setup events if there are any other battlefields
            setupEvents = false

            -- Do not setup npcs if there is another battlefield using the same entry npc
            if self.entryNpcs then
                for _, entryNpc in ipairs(self.entryNpcs) do
                    if utils.contains(entryNpc, content.entryNpcs) then
                        setupEntryNpcs = false

                        break
                    end
                end
            end
        end
    end

    local zoneSection = self.sections[1][self.zoneId]
    if setupEvents then
        utils.append(zoneSection, {
            onEventUpdate =
            {
                [32000] = Battlefield.redirectEventUpdate,
                [32003] = utils.bind(Battlefield.redirectEventCall, 'onExitEventUpdate'),
            },

            onEventFinish =
            {
                [32000] = utils.bind(Battlefield.redirectEventCall, 'onEventFinishEnter'),
                [32001] = utils.bind(Battlefield.redirectEventCall, 'onEventFinishWin'),
                [32002] = utils.bind(Battlefield.redirectEventCall, 'onEventFinishLeave'),
                [32003] = utils.bind(Battlefield.redirectEventCall, 'onEventFinishExit'),
                [32004] = utils.bind(Battlefield.redirectEventCall, 'onEventFinishBattlefield'),
                [32005] = utils.bind(Battlefield.redirectEventCall, 'onEventFinishBattlefield'),
            }
        })
        self.hasListeners = true
    end

    if setupEntryNpcs and self.entryNpcs then
        for _, entryNpc in ipairs(self.entryNpcs) do
            utils.append(zoneSection, {
                [entryNpc] =
                {
                    onTrade   = Battlefield.onEntryTrade,
                    onTrigger = Battlefield.onEntryTrigger,
                }
            })
        end
    end

    if self.exitNpcs then
        local exitTrigger = self.onExitTrigger and self.onExitTrigger or Battlefield.onExitTrigger
        for _, exitNpc in ipairs(self.exitNpcs) do
            utils.append(zoneSection, {
                [exitNpc] =
                {
                    onTrigger = exitTrigger,
                }
            })
        end
    end

    xi.battlefield.contents[self.battlefieldId] = self

    if utils.hasKey(self.zoneId, xi.battlefield.contentsByZone) then
        table.insert(xi.battlefield.contentsByZone[self.zoneId], self)
    else
        xi.battlefield.contentsByZone[self.zoneId] = { self }
    end

    return self
end

function Battlefield:isValidEntry(player, npc)
    return utils.contains(npc:getName(), self.entryNpcs)
end

-- Allow for Battlefield scripts to easily add additional requirements for entry by
-- redefining this function
function Battlefield:entryRequirement(player, npc, isRegistrant, trade)
    return true
end

function Battlefield:checkRequirements(player, npc, isRegistrant, trade)
    if not self:isValidEntry(player, npc) then
        return false
    end

    -- Do not show battlefields to registrant when either they don't require items and player is trading or
    -- that do require items and but player is not trading
    if isRegistrant and (trade == nil) ~= (#self.tradeItems == 0) then
        return false
    end

    -- Everyone must hold the required key items by default.
    -- Note : Key items are not consumed here, only checked, consumption is handled in onBattlefieldEnter
    local needsKeyItems = true

    -- Battlefields flagged 'onlyInitiator' only require them from the battlefield initiator to enter.
    if self.requiredKeyItems.onlyInitiator then
        needsKeyItems = isRegistrant
    end

    if needsKeyItems then
        for _, keyItem in ipairs(self.requiredKeyItems) do
            if type(keyItem) == 'table' then

                -- Some battlefields may be entered with one of several key items. (e.g. Requiem of Sin)
                local hasAnyKeyItem = false

                for _, subitem in ipairs(keyItem) do
                    if player:hasKeyItem(subitem) then
                        hasAnyKeyItem = true

                        break
                    end
                end

                if not hasAnyKeyItem then
                    return false
                end

            else
                if not player:hasKeyItem(keyItem) then
                    return false
                end
            end
        end
    end

    if trade and #self.tradeItems > 0 then
        if not npcUtil.tradeHasExactly(trade, self.tradeItems) then
            return false
        end
    end

    -- Additional Requirements that may be necessary for battlefield entry
    -- contained within the script itself, defaults to True
    return self:entryRequirement(player, npc, isRegistrant, trade)
end

function Battlefield:checkSkipCutscene(player)
    return false
end

function Battlefield.onEntryTrade(player, npc, trade, onUpdate)
    -- Check if player's party has level sync
    if xi.battlefield.rejectLevelSyncedParty(player, npc) then
        return
    end

    -- Validate trade
    if not trade then
        return
    end

    -- Validate battlefield status
    if player:hasStatusEffect(xi.effect.BATTLEFIELD) and not onUpdate then
        player:messageBasic(xi.msg.basic.WAIT_LONGER, 0, 0)

        return
    end

    -- Check if another party member has battlefield status effect. If so, don't allow trade.
    local alliance = player:getAlliance()

    for _, member in pairs(alliance) do
        if member:hasStatusEffect(xi.effect.BATTLEFIELD) then
            player:messageBasic(xi.msg.basic.WAIT_LONGER, 0, 0)

            return
        end
    end

    local zoneId = player:getZoneID()

    -- Determine which battlefields are available given the traded items
    local options = xi.battlefield.getBattlefieldOptions(player, npc, trade)

    if options == 0 then
        local noEntryMessage = zones[zoneId].text.NO_BATTLEFIELD_ENTRY

        if noEntryMessage then
            player:messageSpecial(noEntryMessage)
        end

        return
    end

    -- Ensure that the traded item(s) are not worn out
    local contents = xi.battlefield.contentsByZone[zoneId]

    for _, content in ipairs(contents) do
        if
            #content.requiredItems > 0 and
            content.requiredItems.wornMessage and
            npcUtil.tradeHas(trade, content.tradeItems)
        then
            local itemId = content.requiredItems[1]
            -- Gets the total number of item uses for the given item. Default to one since that is the majority of them.
            local totalUses = xi.battlefield.itemUses[itemId] or 1

            if player:getWornUses(itemId) >= totalUses then
                if type(content.requiredItems.wornMessage) == 'table' then
                    player:messageSpecial(unpack(content.requiredItems.wornMessage))
                elseif totalUses > 1 then
                    player:messageSpecial(content.requiredItems.wornMessage, itemId)
                else
                    player:messageSpecial(content.requiredItems.wornMessage, 0, 0, 0, itemId)
                end

                return
            end
        end
    end

    if not onUpdate then
        -- Open menu of valid battlefields
        return Battlefield.openEntryMenu(player, options, false)
    end
end

-- Helper function to open the battlefield entry menu for the player
function Battlefield.openEntryMenu(player, options, enterExisting)
    player:setLocalVar('[BCNM]EnterExisting', enterExisting and 1 or 0)
    player:setLocalVar('[BCNM]MenuOptions', options)

    return Battlefield:event(32000, 0, 0, 0, options, 0, 0, 0, 0)
end

function Battlefield.onEntryTrigger(player, npc)
    -- Cannot enter if anyone in party is level/master sync'd
    if xi.battlefield.rejectLevelSyncedParty(player, npc) then
        return
    end

    -- Player has battlefield status effect. That means a battlefield is open OR the player is inside a battlefield.
    if player:hasStatusEffect(xi.effect.BATTLEFIELD) then
        -- Player is outside battlefield. Attempting to enter.
        local status  = player:getStatusEffect(xi.effect.BATTLEFIELD)
        local id      = status:getPower()
        local content = xi.battlefield.contents[id]
        if not content then
            return
        end

        if not content:checkRequirements(player, npc, false) then
            return
        end

        local options = utils.mask.setBit(0, content.index, true)
        return Battlefield.openEntryMenu(player, options, true)
    end

    -- Player doesn't have battlefield status effect. That means player wants to register a new battlefield OR is attempting to enter a closed one.
    -- Check if another party member has battlefield status effect. If so, that means the player is trying to enter a closed battlefield.
    local ID       = zones[player:getZoneID()]
    local alliance = player:getAlliance()

    for _, member in pairs(alliance) do
        if member:hasStatusEffect(xi.effect.BATTLEFIELD) then
            -- A member registered for the fight lost clearance when it started, anyone else never had it
            local registered = player:getRegisteredBattlefield()

            if registered and registered:getStatus() ~= xi.battlefield.status.OPEN then
                player:messageText(npc, ID.text.CLEARED_BUT_MEMBERS_ENGAGED, false)
            else
                player:messageSpecial(ID.text.PARTY_MEMBERS_ARE_ENGAGED)
            end

            return
        end
    end

    -- No one in party/alliance has battlefield status effect. We want to register a new battlefield.
    local options = xi.battlefield.getBattlefieldOptions(player, npc)

    -- GMs get access to all BCNMs with visible GM
    if player:getGMLevel() > 0 and player:getVisibleGMLevel() >= 3 then
        options = 268435455
    end

    -- options = 268435455 -- uncomment to open menu with all possible battlefields
    if options == 0 then
        local noEntryMessage = zones[player:getZoneID()].text.NO_BATTLEFIELD_ENTRY

        if noEntryMessage then
            player:messageSpecial(noEntryMessage)
        end

        return
    end

    return Battlefield.openEntryMenu(player, options, false)
end

-- Static function to lookup the correct battlefield to handle this event update
function Battlefield.redirectEventUpdate(player, csid, option, npc)
    if option == 0 or option == 255 then
        return false
    end

    local contents = xi.battlefield.contentsByZone[player:getZoneID()]
    local value    = bit.band(bit.rshift(option, 4), 0x1F)

    -- Only the battlefields the menu was opened with can be registered from it
    if not utils.mask.getBit(player:getLocalVar('[BCNM]MenuOptions'), value) then
        player:updateEvent(xi.battlefield.returnCode.REQS_NOT_MET)
        player:setLocalVar('noPosUpdate', 1) -- checked in GP_CLI_COMMAND_EVENTENDXZY

        return false
    end

    for _, content in pairs(contents) do
        if value == content.index then
            content:onEntryEventUpdate(player, csid, option, npc)

            break
        end
    end
end

-- The entry event asks for the zone's arenas in order.
-- Bits 0 to 3 of the option hold the arena the client wants to move into and bits 4 to 8 the menu index.
-- The client tries the next arena on WAIT (or no reply) then restarts from the first arena on INCREMENT_REQUEST.
-- It then prints the matching message and stops on LOCKED, REQS_NOT_MET and BATTLEFIELD_FULL.
-- Registering the arena that was asked for keeps the battlefield and the position the client moves to in sync so we dont have to save it locally anymore
function Battlefield:onEntryEventUpdate(player, csid, option, npc)
    local requestedArea = bit.band(option, 0xF)
    if
        not self.area and
        (requestedArea < 1 or requestedArea > self.maxAreas)
    then
        player:updateEvent(xi.battlefield.returnCode.REQS_NOT_MET)
        player:setLocalVar('noPosUpdate', 1)
        return 0
    end

    local clearTime = 1
    local name      = 'Meme'
    local partySize = 1
    local area      = self.area or requestedArea

    -- allowedAreas disables arenas whose mobs are not placed yet, the client moves on as if they were taken
    if
        self.allowedAreas and
        not self.allowedAreas[area]
    then
        player:updateEvent(xi.battlefield.returnCode.WAIT)
        player:setLocalVar('noPosUpdate', 1)
        return 0
    end

    local result = player:registerBattlefield(self.battlefieldId, area, player:getID())
    local status = xi.battlefield.status.OPEN

    if result ~= xi.battlefield.returnCode.CUTSCENE then
        if result == xi.battlefield.returnCode.INCREMENT_REQUEST then
            result = xi.battlefield.returnCode.WAIT
        elseif result == xi.battlefield.returnCode.REQS_NOT_MET then
            -- If you cant enter now, you cant enter by trying again right now...
            player:delStatusEffect(xi.effect.BATTLEFIELD)
        end

        player:updateEvent(result)

        -- TODO: Remove the localVar when issue with function return is resolved
        player:setLocalVar('noPosUpdate', 1)
        return 0
    end

    -- Only allow entrance if battlefield is open and player has battlefield effect, witch can be lost mid battlefield selection.
    if
        not player:getBattlefield() and
        player:hasStatusEffect(xi.effect.BATTLEFIELD)
        -- and id:getStatus() == xi.battlefield.status.OPEN -- TODO: Uncomment only once that can-of-worms is dealt with.
    then
        player:enterBattlefield()
    end

    -- Handle record
    local initiatorId = 0
    local battlefield = player:getBattlefield()

    if battlefield then
        name, clearTime, partySize = battlefield:getRecord()
        initiatorId, _ = battlefield:getInitiator()
    end

    -- Register party members
    if initiatorId == player:getID() then
        local effect = player:getStatusEffect(xi.effect.BATTLEFIELD)
        local zone   = player:getZoneID()

        -- Handle traded items if not wearing them
        if
            self.requiredItems.wearMessage == nil and
            #self.tradeItems > 0
        then
            if not self.requiredItems.keep then
                player:tradeComplete()
            end
        end

        -- Handle party/alliance members
        local alliance = player:getAlliance()
        for _, member in pairs(alliance) do
            if
                member:getZoneID() == zone and
                member:getStatus() ~= xi.status.DISAPPEAR and
                not member:hasStatusEffect(xi.effect.BATTLEFIELD) and
                not member:getBattlefield()
            then
                member:copyStatusEffect(effect)
                member:registerBattlefield(self.battlefieldId, area, player:getID())
            end
        end
    end

    -- The client only skips the cutscene when this equals 100 plus the menu index it is given
    local autoSkipCS = self:getLocalVar(player, 'CS') == 1 and 100 + self.index or 0
    player:updateEvent(result, self.index, autoSkipCS, clearTime, partySize, self:checkSkipCutscene(player), self.csParam7, self.csParam8)
    player:updateEventString(name)

    return (status < xi.battlefield.status.LOCKED and result < xi.battlefield.returnCode.LOCKED) and 1 or 0
end

function Battlefield.redirectEventCall(eventName, player, csid, option)
    local battlefield = player:getBattlefield()

    -- battlefield ids start at 0
    if not battlefield and player:getLocalVar('hasBattlefieldID') == 0 then
        return
    end

    local battlefieldID = battlefield and battlefield:getID() or player:getLocalVar('battlefieldID')
    local content       = xi.battlefield.contents[battlefieldID]
    content[eventName](content, player, csid, option)
end

function Battlefield:onEventFinishEnter(player, csid, option, npc)
    -- The menu also closes for players that were granted no arena
    if not player:getBattlefield() then
        return
    end

    player:setEnteredBattlefield(true)
    self:setLocalVar(player, 'CS', 1)
end

function Battlefield:onEventFinishWin(player, csid, option, npc)
    if self.title then
        player:addTitle(self.title)
    end

    if self.grantXP then
        if self.grantXPLockout then
            if self:getVar(player, 'XP') > GetSystemTime() then
                return
            end

            self:setVar(player, 'XP', JstMidnight())
        end

        player:addExp(self.grantXP)
    end
end

function Battlefield.onExitTrigger(player, npc)
    if player:getBattlefield() then
        return Battlefield:progressOptionalCutscene(32003, { cs_option = 3, canSkip = true })
    end
end

-- The exit event only wants its updates acknowledged, the player leaves on finish
function Battlefield:onExitEventUpdate(player, csid, option, npc)
end

function Battlefield:onEventFinishLeave(player, csid, option, npc)
    player:leaveBattlefield(1)
end

function Battlefield:onEventFinishExit(player, csid, option, npc)
    if option == 4 and player:getBattlefield() then
        player:leaveBattlefield(1)
    end
end

function Battlefield:onEventFinishBattlefield(player, csid, option, npc)
end

-- Override this function if necessary to perform additional steps in battlefield
-- Initialize.
function Battlefield:setupBattlefield(battlefield)
end

function Battlefield:onBattlefieldInitialize(battlefield)
    local hasMultipleAreas = not self.area
    battlefield:addGroups(self.groups, hasMultipleAreas)

    -- NOTE: Experimental battlefields are at most partially implemented.  Increase mob levels
    -- for temporary tuning.
    if self.experimental then
        local battlefieldMobs = battlefield:getMobs(true, true)
        for _, mobObj in ipairs(battlefieldMobs) do
            mobObj:setMobLevel(math.min(mobObj:getMainLvl() * 2, 255))
        end
    end

    for mobId, path in pairs(self.paths) do
        local mEntity = GetMobByID(mobId)
        if mEntity then
            mEntity:pathThrough(path, xi.path.flag.PATROL)
        end
    end

    self:setupBattlefield(battlefield)
end

function Battlefield:onBattlefieldTick(battlefield, tick)
    local status        = battlefield:getStatus()
    local cutsceneTimer = battlefield:getLocalVar('cutsceneTimer')
    local isExiting     = status == xi.battlefield.status.LOST or status == xi.battlefield.status.WON

    if isExiting then
        battlefield:setLocalVar('cutsceneTimer', cutsceneTimer - 1)
    end

    -- Check that players haven't all died or that their dead time is over.
    local players = battlefield:getPlayers()
    self:handleWipe(battlefield, players)

    local hasTimeRemaining = xi.battlefield.SendTimePrompts(battlefield, players)

    if not hasTimeRemaining then
        for _, player in pairs(players) do
            player:messageSpecial(zones[player:getZoneID()].text.TIME_IN_THE_BATTLEFIELD_IS_UP)
        end

        battlefield:setStatus(xi.battlefield.status.LOST)
        isExiting = true
    end

    if isExiting then
        -- Remaining in battlefield for an extended time
        if cutsceneTimer > 0 then
            return
        end

        if status == xi.battlefield.status.LOST then
            for _, player in pairs(players) do
                player:messageSpecial(zones[player:getZoneID()].text.PARTY_MEMBERS_HAVE_FALLEN)
            end
        end

        battlefield:cleanup(true)
    end
end

function Battlefield:onBattlefieldRegister(player, battlefield)
end

function Battlefield:onBattlefieldStatusChange(battlefield, status)
    -- Remove battlefield effect for players in alliance not inside battlefield once the battlefield gets locked. Do this only once.
    if
        status == xi.battlefield.status.LOCKED and
        battlefield:getLocalVar('statusRemoval') == 0
    then
        battlefield:setLocalVar('statusRemoval', 1)
        local players = battlefield:getPlayers()

        for _, player in pairs(players) do
            local alliance = player:getAlliance()

            for _, member in pairs(alliance) do
                if
                    member:hasStatusEffect(xi.effect.BATTLEFIELD) and
                    not member:getBattlefield()
                then
                    member:delStatusEffect(xi.effect.BATTLEFIELD)
                end
            end
        end
    end
end

---@diagnostic disable-next-line: duplicate-set-field
function Battlefield:battlefieldEntry(player, battlefield)
end

---@diagnostic disable-next-line: duplicate-set-field
function Battlefield:onBattlefieldEnter(player, battlefield)
    player:setLocalVar('battlefieldID', battlefield:getID())
    player:setLocalVar('hasBattlefieldID', 1)

    local initiatorId, _ = battlefield:getInitiator()

    -- Handle key item consumption. Runs for every player that enters the battlefield.
    -- If onlyInitiator is true, then only the player that registered the battlefield will have their key item consumed.
    if
        #self.requiredKeyItems > 0 and
        (not self.requiredKeyItems.onlyInitiator or player:getID() == initiatorId)
    then

        local items = {}

        for _, item in ipairs(self.requiredKeyItems) do
            -- If this item is a table then that means we only need one of them so delete the first one we find
            if type(item) == 'table' then
                for _, subitem in ipairs(item) do
                    if player:hasKeyItem(subitem) then
                        if not self.requiredKeyItems.keep then
                            player:delKeyItem(subitem)
                        end

                        table.insert(items, subitem)
                    end
                end
            else
                if not self.requiredKeyItems.keep then
                    player:delKeyItem(item)
                end

                table.insert(items, item)
            end
        end

        if type(self.requiredKeyItems.message) == 'table' then
            player:messageSpecial(self.requiredKeyItems.message[1], unpack(self.requiredKeyItems.message[2]))
        elseif self.requiredKeyItems.message then
            player:messageSpecial(self.requiredKeyItems.message, unpack(items))
        end

        if not self.requiredKeyItems.keep and self.requiredKeyItems.deleteMessage then
            for _, keyItemId in ipairs(items) do
                player:messageSpecial(self.requiredKeyItems.deleteMessage, keyItemId)
            end
        end
    end

    if
        player:getID() == initiatorId and
        self.requiredItems.wearMessage and
        player:hasItem(self.requiredItems[1])
    then
        local itemId    = self.requiredItems[1]
        local uses      = player:incrementItemWear(itemId)
        local totalUses = xi.battlefield.itemUses[itemId] or 1 -- Gets number of item uses. (Tests = 3; Else = 1)

        if totalUses > 1 then
            local remaining = totalUses - uses
            player:messageSpecial(self.requiredItems.wearMessage, itemId, remaining + 1, remaining)
        else
            player:messageSpecial(self.requiredItems.wearMessage, 0, 0, 0, itemId)
        end
    end

    -- TODO: Test on retail to find out...
    -- Do mobs buff inside of BCNMs with long cutscenes?
    -- Do mobs buff when not engaged?
    -- With a second player that has completed the mission as a "helper" to go inside skiping the
    -- CS and watch if the mobs actually dont start casting until the initiator is out of the CS

    --[[
    if player:getID() == initiatorId then
        local mobs = battlefield:getMobs(true, true)
        for _, mob in pairs(mobs) do
            if mob:isSpawned() then
                -- wait until initiator is out of cutscene
                mob:addListener('ROAM_TICK', 'FIRST_CAST', function(mobArg)
                    local firstPlayer = GetPlayerByID(initiatorId)
                    if firstPlayer and not firstPlayer:isInEvent() then
                        mobArg:castSpell()

                        mobArg:removeListener('FIRST_CAST')
                    end
                end)
            end
        end
    end
    --]]

    local ID = zones[self.zoneId]
    player:messageSpecial(ID.text.ENTERING_THE_BATTLEFIELD_FOR, 0, self.index)

    if self.maxPlayers > 6 then
        -- NOTE: Update tooling does not allow for duplicate messages to be stored in IDs.lua, even if the ID is different.
        -- Apollyon does not differentiate between party and alliance for message, so if the entry is nil, use party message.
        local messageId = ID.text.MEMBERS_OF_YOUR_ALLIANCE and ID.text.MEMBERS_OF_YOUR_ALLIANCE or ID.text.MEMBERS_OF_YOUR_PARTY

        player:messageSpecial(messageId, 0, 0, 0, self.maxPlayers)
    else
        player:messageSpecial(ID.text.MEMBERS_OF_YOUR_PARTY, 0, 0, 0, self.maxPlayers)
    end

    player:messageSpecial(ID.text.TIME_LIMIT_FOR_THIS_BATTLE_IS, 0, 0, 0, math.floor(self.timeLimit / 60))

    if self.experimental then
        player:printToPlayer('This battlefield has been marked as experimental.  Enemy levels have increased!', xi.msg.channel.NS_SHOUT)
    end

    self:battlefieldEntry(player, battlefield)
end

---@diagnostic disable-next-line: duplicate-set-field
function Battlefield:onBattlefieldDestroy(battlefield)
end

---@diagnostic disable-next-line: duplicate-set-field
function Battlefield:onBattlefieldLeave(player, battlefield, leavecode)
    if
        self.cleanupDebuffs and
        leavecode == xi.battlefield.leaveCode.WON
    then
        player:delStatusEffectsByFlag(xi.effectFlag.ERASABLE + xi.effectFlag.WALTZABLE)
    end

    if leavecode == xi.battlefield.leaveCode.WON then
        self:onBattlefieldWin(player, battlefield)
    elseif leavecode == xi.battlefield.leaveCode.LOST then
        self:onBattlefieldLoss(player, battlefield)
    end
end

function Battlefield:onBattlefieldWin(player, battlefield)
    local _, clearTime, partySize = battlefield:getRecord()

    player:setLocalVar('battlefieldWin', battlefield:getID())
    player:startEvent(32001, battlefield:getArea(), clearTime, partySize, battlefield:getTimeInside(), player:getZoneID(), self.index, 0)
end

function Battlefield:onBattlefieldLoss(player, battlefield)
    player:startEvent(32002, self.lossEventParams)
end

function Battlefield:onBattlefieldKick(player)
    player:startEvent(32002, self.lossEventParams)
end

function Battlefield:handleWipe(battlefield, players)
    local wipeTime = battlefield:getWipeTime()
    local elapsed  = battlefield:getTimeInside()

    -- If party has not yet wiped.
    if wipeTime <= 0 then
        -- Return if any players are still alive
        for _, player in pairs(players) do
            if player:isAlive() then
                return
            end
        end

        self:onBattlefieldWipe(battlefield, players)

    -- Party has already wiped.
    else
        if (elapsed - wipeTime) > utils.minutes(3) then
            battlefield:setStatus(xi.battlefield.status.LOST)
        else
            -- Cancel the wipe timer if any players are alive once again
            for _, player in pairs(players) do
                if player:isAlive() then
                    battlefield:setWipeTime(0)
                    break
                end
            end
        end
    end
end

function Battlefield:onBattlefieldWipe(battlefield, players)
    -- Party has wiped. Save and send time remaining before being booted.
    if self.hasWipeGrace then
        for _, player in pairs(players) do
            player:messageSpecial(zones[player:getZoneID()].text.THE_PARTY_WILL_BE_REMOVED, 0, 0, 0, 3)
        end

        battlefield:setWipeTime(battlefield:getTimeInside())
    else
        battlefield:setStatus(xi.battlefield.status.LOST)
    end
end

function Battlefield:addEssentialMobs(mobNames)
    table.insert(self.groups, {
        mobs      = mobNames,
        superlink = true,
        isParty   = true,
        allDeath  = utils.bind(self.handleAllMonstersDefeated, self),
    })
end

function Battlefield:handleAllMonstersDefeated(battlefield, mob)
    local crateId = battlefield:getArmouryCrate()

    -- NOTE: Default to core returning us a value, but if that fails, use the fallback
    -- definition if available.  This should only be used where mobIds table has to be
    -- used instead of mobs!
    if crateId == 0 and self.armouryCrates then
        crateId = self.armouryCrates[battlefield:getArea()]
    end

    if crateId ~= 0 then
        local crate = GetNPCByID(crateId)

        if crate then
            battlefield:insertEntity(crate:getTargID(), false, true)
            npcUtil.showCrate(crate)
            crate:addListener('ON_TRIGGER', 'TRIGGER_CRATE', utils.bind(self.handleOpenArmouryCrate, self))
        end
    else
        battlefield:setLocalVar('cutsceneTimer', self.delayToExit)
        battlefield:setStatus(xi.battlefield.status.WON)
    end
end

function Battlefield:handleOpenArmouryCrate(player, npc)
    npcUtil.openCrate(npc, function()
        local battlefield = player:getBattlefield()
        self:handleLootRolls(battlefield, self.loot, npc, player:getMod(xi.mod.MOGHANCEMENT_GIL_BONUS_P))
        battlefield:setStatus(xi.battlefield.status.WON)
        battlefield:setLocalVar('cutsceneTimer', self.delayToExit)

        return true
    end)
end

function Battlefield:handleLootRolls(battlefield, lootTable, npc, gilBonusMod)
    local players = battlefield:getPlayers()
    local firstPlayer = players[1]

    local selectedLoot = utils.selectFromLootGroups(firstPlayer, lootTable)
    for _, entry in ipairs(selectedLoot) do
        if entry.itemId ~= xi.item.GIL then
            firstPlayer:addTreasure(entry.itemId, npc)
        else
            local gilBonusPct  = (100 + gilBonusMod) / 100
            local gilPerPlayer = entry.amount * gilBonusPct / #players

            for _, player in ipairs(players) do
                npcUtil.giveCurrency(player, 'gil', gilPerPlayer, true)
            end
        end
    end
end

function xi.battlefield.getBattlefieldOptions(player, npc, trade)
    local result   = 0
    local contents = xi.battlefield.contentsByZone[player:getZoneID()]

    if contents == nil then
        return result
    end

    -- TODO: if the battlefield is at capacity, the 32000 event should not start, but instead
    -- a separate message is displayed on retail

    for _, content in ipairs(contents) do
        if
            content:checkRequirements(player, npc, true, trade) and
            not player:battlefieldAtCapacity(content.battlefieldId) and
            (xi.settings.map.BCNM_ENABLE_EXPERIMENTAL or not content.experimental)
        then
            result = utils.mask.setBit(result, content.index, true)
        end
    end

    return result
end

function xi.battlefield.rejectLevelSyncedParty(player, npc)
    for _, member in pairs(player:getAlliance()) do
        if member:hasStatusEffect(xi.effect.LEVEL_SYNC) then
            local zoneId = player:getZoneID()
            local ID     = zones[zoneId]

            -- Your party is unable to participate because certain members' levels are restricted
            player:messageText(npc, ID.text.MEMBERS_LEVELS_ARE_RESTRICTED, false)

            return true
        end
    end

    return false
end

BattlefieldMission         = setmetatable({ }, { __index = Battlefield })
BattlefieldMission.__index = BattlefieldMission

---@diagnostic disable-next-line: duplicate-set-field
BattlefieldMission.__eq    = function(m1, m2)
    return m1.name == m2.name
end

-- Creates a new Limbus Battlefield interaction
-- Data takes the additional following keys:
--  - missionArea: The mission area this battlefield is associated with (optional)
--  - mission: The mission this battlefield is associated with (optional)
--  - missionStatusArea: The mission area to retrieve the mission status from. Will default to using the player's nation (optional)
--  - missionStatus: The optional extra status information xi.mission.status (optional)
--  - requiredMissionStatus: The required mission status to enter
--  - skipMissionStatus: The required mission status to skip the cutscene. Defaults to the required mission status.
---@diagnostic disable-next-line: duplicate-set-field
function BattlefieldMission:new(data)
    local obj = Battlefield:new(data)

    setmetatable(obj, self)
    obj.missionArea           = data.missionArea
    obj.mission               = data.mission
    obj.missionStatusArea     = data.missionStatusArea
    obj.missionStatus         = data.missionStatus
    obj.requiredMissionStatus = data.requiredMissionStatus
    obj.skipMissionStatus     = data.skipMissionStatus or data.requiredMissionStatus
    obj.requiredVar           = data.requiredVar
    obj.requiredValue         = data.requiredValue

    return obj
end

---@diagnostic disable-next-line: duplicate-set-field
function BattlefieldMission:checkRequirements(player, npc, isRegistrant, trade)
    if not Battlefield.checkRequirements(self, player, npc, isRegistrant, trade) then
        return false
    end

    local missionArea = self.missionArea or player:getNation()
    local current     = player:getCurrentMission(missionArea)

    if self.requiredMissionStatus ~= nil then
        local missionStatusArea = self.missionStatusArea or player:getNation()
        local status            = player:getMissionStatus(missionStatusArea, self.missionStatus)

        return (not isRegistrant and (player:hasCompletedMission(missionArea, self.mission) or
            (current == self.mission and status >= self.requiredMissionStatus))) or
            (current == self.mission and status == self.requiredMissionStatus)
    elseif self.requiredVar ~= nil then
        -- COP Missions use a charVar to track since missionStatus is used to define menu settings.
        -- When converting these, allow for the same structure of status, but with defined requiredVar (str)
        -- and requiredValue (uint requirement).  These are currently mutually exclusive, and treated
        -- as such.

        local status = player:getCharVar(self.requiredVar)

        return (not isRegistrant and (player:hasCompletedMission(missionArea, self.mission) or
            (current == self.mission and status >= self.requiredValue))) or
            (current == self.mission and status == self.requiredValue)
    else
        return (not isRegistrant and player:hasCompletedMission(missionArea, self.mission)) or
            current == self.mission
    end
end

---@diagnostic disable-next-line: duplicate-set-field
function BattlefieldMission:checkSkipCutscene(player)
    local missionArea       = self.missionArea or player:getNation()
    local current           = player:getCurrentMission(missionArea)
    local missionStatusArea = self.missionStatusArea or player:getNation()
    local status            = player:getMissionStatus(missionStatusArea, self.missionStatus)

    return player:hasCompletedMission(missionArea, self.mission) or
        (self.requiredMissionStatus and current == self.mission and status > self.skipMissionStatus) or
        (self.requiredValue and current == self.mission and player:getCharVar(self.requiredVar) > self.requiredValue)
end

---@diagnostic disable-next-line: duplicate-set-field
function BattlefieldMission:onBattlefieldWin(player, battlefield)
    local missionArea       = self.missionArea or player:getNation()
    local current           = player:getCurrentMission(missionArea)
    local missionStatusArea = self.missionStatusArea or player:getNation()

    if current == self.mission then
        player:setLocalVar('battlefieldWin', battlefield:getID())
    end

    local _, clearTime, partySize = battlefield:getRecord()
    local canSkipCS               = (player:hasCompletedMission(missionArea, self.mission) or
        (self.requiredMissionStatus and current == self.mission and player:getMissionStatus(missionStatusArea, self.missionStatus) > self.skipMissionStatus) or
        (self.requiredValue and current == self.mission and player:getCharVar(self.requiredVar) > self.requiredValue)) and 1 or 0

    player:startEvent(32001, battlefield:getArea(), clearTime, partySize, battlefield:getTimeInside(), player:getZoneID(), self.index, canSkipCS)
end

---@diagnostic disable-next-line: duplicate-set-field
function BattlefieldMission:onEventFinishWin(player, csid, option, npc)
    if self.title then
        player:addTitle(self.title)
    end

    -- Only grant mission XP once per JP midnight
    if self.grantXP and self:getVar(player, 'XP') <= GetSystemTime() then
        self:setVar(player, 'XP', JstMidnight())
        player:addExp(self.grantXP)
    end
end

BattlefieldQuest         = setmetatable({ }, { __index = Battlefield })
BattlefieldQuest.__index = BattlefieldQuest

---@diagnostic disable-next-line: duplicate-set-field
BattlefieldQuest.__eq    = function(m1, m2)
    return m1.name == m2.name
end

BattlefieldQuest.isQuest = true

-- Creates a new Limbus Battlefield interaction
-- Data takes the additional following keys:
--  - questArea: The quest area this battlefield is associated with (optional)
--  - quest: The quest this battlefield is associated with (optional)
---@diagnostic disable-next-line: duplicate-set-field
function BattlefieldQuest:new(data)
    local obj = Battlefield:new(data)
    setmetatable(obj, self)

    obj.questArea     = data.questArea
    obj.quest         = data.quest
    obj.requiredVar   = data.requiredVar
    obj.requiredValue = data.requiredValue

    return obj
end

---@diagnostic disable-next-line: duplicate-set-field
function BattlefieldQuest:checkRequirements(player, npc, isRegistrant, trade)
    if not Battlefield.checkRequirements(self, player, npc, isRegistrant, trade) then
        return false
    end

    if self.requiredVar ~= nil then
        -- COP Missions use a charVar to track since missionStatus is used to define menu settings.
        -- When converting these, allow for the same structure of status, but with defined requiredVar (str)
        -- and requiredValue (uint requirement).  These are currently mutually exclusive, and treated
        -- as such.

        local status      = player:getCharVar(self.requiredVar)
        local hasAccepted = player:getQuestStatus(self.questArea, self.quest) == xi.questStatus.QUEST_ACCEPTED

        return (not isRegistrant and (player:hasCompletedQuest(self.questArea, self.quest) or
            (hasAccepted and status >= self.requiredValue))) or
            (hasAccepted and status == self.requiredValue)
    else
        return player:getQuestStatus(self.questArea, self.quest) >= xi.questStatus.QUEST_ACCEPTED
    end
end

---@diagnostic disable-next-line: duplicate-set-field
function BattlefieldQuest:checkSkipCutscene(player)
    return player:getQuestStatus(self.questArea, self.quest) == xi.questStatus.QUEST_COMPLETED
end

---@diagnostic disable-next-line: duplicate-set-field
function BattlefieldQuest:onBattlefieldWin(player, battlefield)
    local status = player:getQuestStatus(self.questArea, self.quest)

    if status == xi.questStatus.QUEST_ACCEPTED then
        player:setLocalVar('battlefieldWin', battlefield:getID())
    end

    local _, clearTime, partySize = battlefield:getRecord()
    local canSkipCS               = status ~= xi.questStatus.QUEST_ACCEPTED and 1 or 0

    player:startEvent(32001, battlefield:getArea(), clearTime, partySize, battlefield:getTimeInside(), player:getZoneID(), self.index, canSkipCS)
end

-- returns false if out of time
function xi.battlefield.SendTimePrompts(battlefield, players)
    local remainingTime  = battlefield:getRemainingTime()
    local lastTimeUpdate = battlefield:getLastTimeUpdate()
    local timeLimit      = battlefield:getTimeLimit()
    local message        = 0

    players = players or battlefield:getPlayers()

    if remainingTime < 600 and lastTimeUpdate > 600 and timeLimit > 600 then
        message = 600
    elseif remainingTime < 300 and lastTimeUpdate > 300 and timeLimit > 300 then
        message = 300
    elseif remainingTime < 60 and lastTimeUpdate > 60 and timeLimit > 60 then
        message = 60
    elseif remainingTime < 30 and lastTimeUpdate > 30 and timeLimit > 30 then
        message = 30
    elseif remainingTime < 10 and lastTimeUpdate > 10 and timeLimit > 10 then
        message = 10
    end

    if message ~= 0 then
        for i, player in pairs(players) do
            player:messageBasic(xi.msg.basic.TIME_LEFT, message)
        end

        battlefield:setLastTimeUpdate(message)
    end

    return remainingTime > 0
end
