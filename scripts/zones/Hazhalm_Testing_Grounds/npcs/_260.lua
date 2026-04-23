-----------------------------------
-- Area: Hazhalm_Testing_Grounds
-- NPC: Entry Gate (_260)
-----------------------------------
local ID = zones[xi.zone.HAZHALM_TESTING_GROUNDS]
-----------------------------------
---@type TNpcEntity
local entity = {}

entity.declaredTrades =
{
    {
        match   = { items = {{ xi.item.SMOLDERING_LAMP, 1 }} },
        acceptIf = function(player, npc)
            return xi.einherjar.settings.EINHERJAR_ENABLED
                and xi.einherjar.meetsRequirementsForReservation(player)
        end,
        event = {
            id     = 2,
            params = function(player)
                return {
                    0,
                    xi.besieged.getMercenaryRank(player),
                    xi.einherjar.settings.EINHERJAR_KO_EXPEL_TIME,
                    xi.einherjar.settings.EINHERJAR_REENTRY_TIME,
                    0,
                    xi.einherjar.getChambersMenu(player),
                    xi.item.SMOLDERING_LAMP,
                    xi.item.GLOWING_LAMP,
                }
            end,
            onFinish = function(player, option, npc)
                -- Chamber-range options (65..74) signal reservation success.
                if option >= 65 and option <= 74 then
                    player:messageSpecial(ID.text.GLOWING_LAMP_OBTAINED, xi.item.GLOWING_LAMP)
                    player:messageSpecial(ID.text.CLAIM_RELINQUISH, xi.item.GLOWING_LAMP, xi.einherjar.settings.EINHERJAR_RESERVATION_TIMEOUT)
                    player:messageSpecial(ID.text.ITEM_OBTAINED, xi.item.GLOWING_LAMP)
                    return true
                end
                return false
            end,
        },
    },

    {
        match   = { items = {{ xi.item.GLOWING_LAMP, 1 }} },
        acceptIf = function(player, npc)
            if not xi.einherjar.settings.EINHERJAR_ENABLED then
                return false
            end
            -- Chamber id lives in lamp exdata; stash on localVars so
            -- the onFinish handler can use it after the event.
            local lampObj  = player:findItem(xi.item.GLOWING_LAMP)
            if not lampObj then
                return false
            end
            local lampData = xi.einherjar.decypherLamp(lampObj)
            local chamber  = xi.einherjar.getChamber(lampData.chamberId)
            if not chamber then
                xi.einherjar.voidLamp(player, lampObj)
                player:messageSpecial(ID.text.REQUIREMENTS_UNMET)
                return false
            end
            if not xi.einherjar.meetsRequirementsForEntry(player, lampData.chamberId) then
                return false
            end
            player:setLocalVar('[ein]requestedChamber', lampData.chamberId)
            player:setLocalVar('[ein]requestedStart', lampData.startTime)
            return true
        end,
        event = {
            id     = 3,
            params = function(player)
                local chamberId = player:getLocalVar('[ein]requestedChamber')
                return {
                    0x1D + chamberId,
                    xi.besieged.getMercenaryRank(player),
                    xi.einherjar.settings.EINHERJAR_KO_EXPEL_TIME,
                    xi.einherjar.settings.EINHERJAR_REENTRY_TIME,
                    0,
                    xi.einherjar.getChambersMenu(player),
                    xi.item.SMOLDERING_LAMP,
                    xi.item.GLOWING_LAMP,
                }
            end,
            onFinish = function(player, option, npc)
                if option == 1 then
                    local chamberId = player:getLocalVar('[ein]requestedChamber')
                    local startTime = player:getLocalVar('[ein]requestedStart')
                    player:setLocalVar('[ein]requestedChamber', 0)
                    player:setLocalVar('[ein]requestedStart', 0)
                    if chamberId ~= 0 and startTime ~= 0 then
                        local chamber = xi.einherjar.getChamber(chamberId)
                        if chamber and chamber.startTime == startTime then
                            xi.einherjar.onChamberEnter(chamber, player)
                        else
                            player:messageSpecial(ID.text.COULD_NOT_GATHER_DATA)
                        end
                    end
                end
                -- Passthrough: lamp returns to the player regardless.
                return false
            end,
        },
    },
}

entity.onTrigger = function(player, npc)
    -- TODO: Entry point for The Rider Cometh
    -- If The Rider Cometh is flagged, no lockout message will show
    -- but the battlefield selection menu will show up
    local lockout = xi.einherjar.isLockedOut(player)
    if lockout ~= 0 then
        player:messageSpecial(ID.text.ENTRY_PROHIBITED, lockout)
        return
    end

    player:messageSpecial(ID.text.GATE_FIRMLY_CLOSED)
end

entity.onEventUpdate = function(player, csid, option, npc)
    if csid == 2 and (option >= 1 and option <= 10) then
        local mask = xi.einherjar.getChambersMenu(player)
        local chamberEntry = xi.einherjar.chambers[option]

        if not chamberEntry or bit.band(mask, chamberEntry.menu) ~= 0 then
            print(string.format("Einherjar: %s attempted to reserve a chamber they don't have access to.", player:getName()))
            player:messageSpecial(ID.text.COULD_NOT_GATHER_DATA)
            player:instanceEntry(npc, 3)
            return
        end

        player:updateEvent(0,
            10,
            xi.einherjar.settings.EINHERJAR_KO_EXPEL_TIME,
            xi.einherjar.settings.EINHERJAR_REENTRY_TIME,
            0,
            xi.einherjar.getChambersMenu(player),
            xi.item.SMOLDERING_LAMP,
            xi.item.GLOWING_LAMP
        )
        if player:getFreeSlotsCount() ~= 0 then
            local chamberData = xi.einherjar.getChamber(option)
            if chamberData then
                player:instanceEntry(npc, 3) -- 3 == chamber reservation failed
                player:messageSpecial(ID.text.CHAMBER_OCCUPIED, option)
                return
            else
                chamberData = xi.einherjar.createNewChamber(option, player)
                if not chamberData then
                    player:messageSpecial(ID.text.COULD_NOT_GATHER_DATA)
                    player:instanceEntry(npc, 3)
                    return
                end
            end

            xi.einherjar.makeLamp(player, chamberData.id, chamberData.startTime, chamberData.endTime)
            xi.einherjar.recordLockout(player)
            player:instanceEntry(npc, 4)
        else
            player:messageSpecial(ID.text.ITEM_CANNOT_BE_OBTAINED, xi.item.GLOWING_LAMP)
            player:instanceEntry(npc, 3)
        end
    end
end

return entity
