-- Collection of retail CHAR_NPC packets for testing
-- Please follow this process:
-- - Add a new entry in the table with an empty test
-- - Name the entry in a descriptive fashion, including the SubKind type
-- - Use 'captain' or packet capture tool to get a dump of the 0x00E packet
-- - Replace the entity ID fields with TEST_MOB/TEST_NPC (adjust accordingly)
-- - Replace dynamic values with IGNORE (for example position values)
-- - Fill in the actual test function

local ph = require('scripts.tests.packets.s2c.0x00e_char_npc.placeholders')

---@class CharNpcSendFlg
---@field Position integer? Position flag
---@field ClaimStatus integer? Claim status flag
---@field General integer? General flag
---@field Name integer? Name flag
---@field Model integer? Model flag
---@field Despawn integer? Despawn flag
---@field Name2 integer? Name2 flag

---@class CharNpcFlags1
---@field raw integer? Full 32-bit flags1 value
---@field MonsterFlag integer? Monster flag
---@field HideFlag integer? Hide flag
---@field SleepFlag integer? Sleep flag
---@field ChocoboIndex integer? Chocobo index
---@field CliPosInitFlag integer? Client position init flag
---@field GraphSize integer? Graphics size
---@field LfgFlag integer? Looking for group flag
---@field AnonymousFlag integer? Anonymous flag
---@field YellFlag integer? Yell flag
---@field AwayFlag integer? Away flag
---@field Gender integer? Gender flag
---@field PlayOnelineFlag integer? Play online flag
---@field LinkShellFlag integer? Linkshell flag
---@field LinkDeadFlag integer? Link dead flag
---@field TargetOffFlag integer? Target off flag
---@field TalkUcoffFlag integer? Talk cutoff flag
---@field GmLevel integer? GM level
---@field HackMove integer? Hack move flag
---@field InvisFlag integer? Invisible flag
---@field TurnFlag integer? Turn flag
---@field BazaarFlag integer? Bazaar flag

---@class CharNpcFlags0
---@field MovTime integer? Movement time
---@field RunMode integer? Run mode flag
---@field unknown_1_6 integer? Unknown (TargetMode)
---@field GroundFlag integer? Ground flag
---@field KingFlag integer? King flag
---@field facetarget integer? Face target ID

---@class CharNpcFlags2
---@field r integer? Red color
---@field g integer? Green color
---@field b integer? Blue color
---@field PvPFlag integer? PvP flag
---@field ShadowFlag integer? Shadow flag
---@field ShipStartMode integer? Ship start mode flag
---@field CharmFlag integer? Charm flag
---@field GmIconFlag integer? GM icon flag
---@field NamedFlag integer? Named flag
---@field SingleFlag integer? Single flag
---@field AutoPartyFlag integer? Auto party flag

---@class CharNpcFlags3
---@field TrustFlag integer? Trust flag
---@field LfgMasterFlag integer? LFG master flag
---@field PetNewFlag integer? Pet new flag
---@field unknown_0_3 integer? Unknown (PetKillFlag)
---@field MotStopFlag integer? Motion stop flag
---@field CliPriorityFlag integer? Client priority flag
---@field PetFlag integer? Pet flag (triggerable)
---@field OcclusionoffFlag integer? Occlusion off flag
---@field BallistaTeam integer? Ballista team
---@field MonStat integer? Monster status
---@field unknown_2_3 integer? Unknown
---@field unknown_2_4 integer? Unknown
---@field SilenceFlag integer? Silence flag
---@field unknown_2_6 integer? Unknown
---@field NewCharacterFlag integer? New character flag
---@field MentorFlag integer? Mentor flag
---@field unknown_3_1 integer? Unknown
---@field unknown_3_2 integer? Unknown
---@field unknown_3_3 integer? Unknown
---@field unknown_3_4 integer? Unknown
---@field unknown_3_5 integer? Unknown
---@field unknown_3_6 integer? Unknown
---@field unknown_3_7 integer? Unknown

---@class CharNpcPacket
---@field UniqueNo integer? Entity server ID
---@field ActIndex integer? Entity target index
---@field SendFlg CharNpcSendFlg? Send flags
---@field dir integer? Direction (0-255)
---@field x number? X coordinate
---@field z number? Z coordinate
---@field y number? Y coordinate
---@field Speed integer? Movement speed
---@field SpeedBase integer? Base movement speed
---@field Hpp integer? HP percentage (0-100)
---@field server_status integer? Server status byte
---@field BtTargetID integer? Battle target ID
---@field SubKind integer? SubKind (0-7)
---@field Status integer? Entity status
---@field Flags0 CharNpcFlags0? Flags0 bitfields
---@field Flags1 CharNpcFlags1? Flags1 bitfields
---@field Flags2 CharNpcFlags2? Flags2 bitfields
---@field Flags3 CharNpcFlags3? Flags3 bitfields
---@field ModelId integer? Model ID (SubKind 0, 5, 6)
---@field Name string? Entity name
---@field GrapIDTbl integer[]? Equipment slots (SubKind 1, 7)
---@field DoorId integer? Door ID (SubKind 2, 3, 4)
---@field Time integer? Animation time (SubKind 3, 4)
---@field EndTime integer? Animation end time (SubKind 3, 4)

---@class CharNpcTestEntry
---@field test fun(player: CClientEntityPair) Test function
---@field expected CharNpcPacket|CharNpcPacket[] Expected packet(s)

---@alias CharNpcTestSuite table<string, CharNpcTestEntry>

---@type table<string, CharNpcTestSuite>
local testSuites =
{
    ['FixedModel']   = require('scripts.tests.packets.s2c.0x00e_char_npc.fixed_model'),
    ['DivingMobs']   = require('scripts.tests.packets.s2c.0x00e_char_npc.diving_mobs'),
    ['Elevator']     = require('scripts.tests.packets.s2c.0x00e_char_npc.elevator'),
    ['Door']         = require('scripts.tests.packets.s2c.0x00e_char_npc.door'),
    ['Equipped']     = require('scripts.tests.packets.s2c.0x00e_char_npc.equipped'),
    -- ['Airship']      = require('scripts.tests.packets.s2c.0x00e_char_npc.airship'),
    -- ['MiscNpc']      = require('scripts.tests.packets.s2c.0x00e_char_npc.misc_npc'),
    -- ['Automaton']    = require('scripts.tests.packets.s2c.0x00e_char_npc.automaton'),
    -- ['EquippedMisc'] = require('scripts.tests.packets.s2c.0x00e_char_npc.equipped_misc'),
}

-- TODO: Replace with some higher level diff library
local function diffPacket(actual, expected, prefix, phValues)
    prefix = prefix or 'charNpc'
    local diffs = {}

    for key, expectedValue in pairs(expected) do
        local actualValue = actual[key]
        local fieldName   = prefix .. '.' .. key

        if type(expectedValue) == 'table' then
            if type(actualValue) ~= 'table' then
                table.insert(diffs, string.format('%s: expected table, got %s', fieldName, type(actualValue)))
            else
                -- Recursively diff
                local nestedDiffs = diffPacket(actualValue, expectedValue, fieldName, phValues)
                for _, diff in ipairs(nestedDiffs) do
                    table.insert(diffs, diff)
                end
            end
        elseif expectedValue == ph.TEST_CHAR then
            -- Match dynamically the test char ID
            if actualValue ~= phValues.charId then
                table.insert(diffs, string.format('%s: %s != %s', fieldName, tostring(actualValue), tostring(phValues.charId)))
            end
        elseif actualValue ~= expectedValue then
            -- Ignore nil expected values (IGNORE placeholder)
            if expectedValue ~= nil then
                table.insert(diffs, string.format('%s: %s != %s', fieldName, tostring(actualValue), tostring(expectedValue)))
            end
        end
    end

    return diffs
end

describe('CHAR_NPC', function()
    ---@type CClientEntityPair
    local player

    before_each(function()
        xi.test.world:setSeed(1)

        player = xi.test.world:spawnPlayer(
            {
                job   = xi.job.WAR,
                level = 99,
                zone  = xi.zone.QUFIM_ISLAND,
            })
    end)

    -- Iterate through test suites in deterministic order
    local suiteNames = {}
    for suiteName in pairs(testSuites) do
        table.insert(suiteNames, suiteName)
    end

    table.sort(suiteNames)

    for _, suiteName in ipairs(suiteNames) do
        local suiteDefinition = testSuites[suiteName]
        describe(suiteName, function()
            -- Iterate through test cases in deterministic order
            local caseNames = {}
            for caseName in pairs(suiteDefinition) do
                table.insert(caseNames, caseName)
            end

            table.sort(caseNames)

            for _, caseName in ipairs(caseNames) do
                local caseDefinition = suiteDefinition[caseName]
                it(caseName, function()
                    caseDefinition.test(player)
                    xi.test.world:tick()

                    -- Determine if we expect one packet or multiple
                    local expectedPackets = {}
                    if caseDefinition.expected[1] == nil then
                        -- Single packet (no array index [1])
                        expectedPackets = { caseDefinition.expected }
                    else
                        -- Multiple packets (array with index [1])
                        expectedPackets = caseDefinition.expected
                    end

                    -- Track which expected packets have been matched
                    local matchedExpected = {}
                    for i = 1, #expectedPackets do
                        matchedExpected[i] = false
                    end

                    local receivedPackets = player.packets:charNpcPackets()
                    local receivedPacketCount = #receivedPackets
                    local allDiffs            = {}

                    for i, npcPacket in pairs(receivedPackets) do
                        -- Try to match against any unmatched expected packet
                        local bestMatchIdx   = nil
                        local bestMatchDiffs = nil

                        for expectedIdx, expectedPacket in pairs(expectedPackets) do
                            if not matchedExpected[expectedIdx] then
                                local diffs = diffPacket(npcPacket, expectedPacket, nil,
                                    { charId = player:getID() })
                                if #diffs == 0 then
                                    InfoTest(string.format('Matched expected packet #%d', expectedIdx))
                                    matchedExpected[expectedIdx] = true
                                    bestMatchIdx                 = expectedIdx
                                    bestMatchDiffs               = diffs
                                    break
                                elseif bestMatchIdx == nil or #diffs < #bestMatchDiffs then
                                    -- Track the closest match for debugging
                                    bestMatchIdx   = expectedIdx
                                    bestMatchDiffs = diffs
                                end
                            end
                        end

                        -- Store diffs for the best match if we didn't find a perfect match
                        if bestMatchIdx ~= nil then
                            allDiffs[i] =
                            {
                                expectedIdx = bestMatchIdx,
                                diffs       = bestMatchDiffs,
                            }
                        end
                    end

                    -- Check if all expected packets were matched
                    local unmatchedPackets = {}
                    for i = 1, #expectedPackets do
                        if not matchedExpected[i] then
                            table.insert(unmatchedPackets, i)
                        end
                    end

                    if #unmatchedPackets > 0 then
                        InfoTest(string.format('Received %d CHAR_NPC packet(s), could not match %d of %d expected packet(s)',
                            receivedPacketCount, #unmatchedPackets, #expectedPackets))

                        -- Build set of expected UniqueNo values for filtering
                        local expectedUniqueNos = {}
                        for _, expectedPacket in pairs(expectedPackets) do
                            if expectedPacket.UniqueNo then
                                expectedUniqueNos[expectedPacket.UniqueNo] = true
                            end
                        end

                        -- Log received packets matching expected UniqueNo values
                        local filteredCount = 0
                        for i, pkt in pairs(receivedPackets) do
                            if expectedUniqueNos[pkt.UniqueNo] then
                                filteredCount = filteredCount + 1
                                InfoTest(string.format('  Packet #%d: UniqueNo=%d, SubKind=%d, server_status=%s',
                                    i,
                                    pkt.UniqueNo or 0,
                                    pkt.SubKind or -1,
                                    tostring(pkt.server_status or 'nil')))
                                InfoTest(string.format('    Flags0: KingFlag=%s, MovTime=%s',
                                    tostring(pkt.Flags0 and pkt.Flags0.KingFlag or 'nil'),
                                    tostring(pkt.Flags0 and pkt.Flags0.MovTime or 'nil')))
                                InfoTest(string.format('    Flags1: TargetOffFlag=%s, TalkUcoffFlag=%s, CliPosInitFlag=%s, GraphSize=%s',
                                    tostring(pkt.Flags1 and pkt.Flags1.TargetOffFlag or 'nil'),
                                    tostring(pkt.Flags1 and pkt.Flags1.TalkUcoffFlag or 'nil'),
                                    tostring(pkt.Flags1 and pkt.Flags1.CliPosInitFlag or 'nil'),
                                    tostring(pkt.Flags1 and pkt.Flags1.GraphSize or 'nil')))
                                if pkt.Data then
                                    InfoTest(string.format('    Data: DoorId=%s, EndTime=%s',
                                        tostring(pkt.Data.DoorId or 'nil'),
                                        tostring(pkt.Data.EndTime or 'nil')))
                                end
                            end
                        end
                        if filteredCount == 0 then
                            InfoTest('  (No packets matched expected UniqueNo values)')
                        end

                        -- For each unmatched expected packet, print diffs for matching UniqueNo only
                        for _, expectedIdx in ipairs(unmatchedPackets) do
                            local expectedUniqueNo = expectedPackets[expectedIdx].UniqueNo
                            local hasMatches = false

                            -- Print diffs only for received packets with matching UniqueNo
                            for recvIdx, pkt in pairs(receivedPackets) do
                                if pkt.UniqueNo == expectedUniqueNo then
                                    local diffs = diffPacket(pkt, expectedPackets[expectedIdx], nil,
                                        { charId = player:getID() })
                                    hasMatches = true
                                    InfoTest(string.format('Expected #%d vs Received packet #%d: %d diffs',
                                        expectedIdx, recvIdx, #diffs))
                                    for _, diff in pairs(diffs) do
                                        InfoTest('  ' .. diff)
                                    end
                                end
                            end

                            if not hasMatches then
                                InfoTest(string.format('Expected #%d (UniqueNo=%d): No received packets with matching UniqueNo',
                                    expectedIdx, expectedUniqueNo or 0))
                            end
                        end

                        assert(false, string.format('Could not match expected CHAR_NPC packet(s): %s',
                            table.concat(unmatchedPackets, ', ')))
                    end
                end)
            end
        end)
    end
end)
