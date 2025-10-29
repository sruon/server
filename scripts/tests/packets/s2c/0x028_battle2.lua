-- Collection of retail BATTLE2 packets for testing
-- Please follow this process:
-- - Add a new entry in the table with an empty test
-- - Name the entry in a descriptive fashion, including the type of action packet.
-- - Use 'actionparse' on Ashita to get a dump of the action packet
-- - Obtain several copies of the packet to figure out uncleared buffers we don't care about
-- - Click the 'Export' button in actionparse and paste the table below
-- - Zero out retail uncleared buffers
-- - Replace the m_uID fields with 20000000 (adjust accordingly if test involves several entities)
-- - For spells you will likely need to update the root info with the test character recast time
-- - Replace the various fields with the appropriate enums
--   - Add the entries if they're missing
-- - Fill in the actual test function

-- TODO: Add test for AoE items with Action V2 (see item_state.cpp ItemStart/ItemInterrupt factories)

local packets =
{
    ['Cure Self (Magic Start)'] =
    {
        ---@param player CClientEntityPair
        test = function(player)
            player:changeJob(xi.job.WHM)
            player:setLevel(99)
            player:addSpell(xi.magic.spell.CURE)
            player.actions:useSpell(player, xi.magic.spell.CURE)
        end,
        expected =
        {
            m_uID   = 20000000,
            trg_sum = 1,
            res_sum = 0,
            cmd_no  = xi.battle.category.MAGIC_START,
            cmd_arg = xi.battle.fourCC.WHITE_MAGIC_CAST,
            info    = 0,
            target  =
            {
                {
                    m_uID      = 20000000,
                    result_sum = 1,
                    result     =
                    {
                        {
                            miss          = xi.battle.resolution.HIT,
                            kind          = 0, -- Retail has uncleared data here
                            sub_kind      = 0, -- Retail has uncleared data here
                            info          = 0, -- Retail has uncleared data here
                            scale         = 0,
                            value         = xi.magic.spell.CURE,
                            message       = 327,
                            bit           = 0,
                            has_proc      = false,
                            proc_kind     = 0,
                            proc_info     = 0,
                            proc_value    = 0,
                            proc_message  = 0,
                            has_react     = false,
                            react_kind    = 0,
                            react_info    = 0,
                            react_value   = 0,
                            react_message = 0,
                        },
                    },
                },
            },
        },
    },
    ['Cure Self (Magic Start (Interrupt))'] =
    {
        test = function(player)
            player:changeJob(xi.job.WHM)
            player:setLevel(99)
            player:addSpell(xi.magic.spell.CURE_IV)
            player.actions:useSpell(player, xi.magic.spell.CURE_IV)
            xi.test.world:tickEntity(player) -- Start the cast
            player.actions:move(10, 10, 10)
            xi.test.world:skipTime(10)       -- Let cast complete for interrupt to process
        end,
        expected =
        {
            m_uID   = 20000000,
            trg_sum = 1,
            res_sum = 0,
            cmd_no  = xi.battle.category.MAGIC_START,
            cmd_arg = xi.battle.fourCC.WHITE_MAGIC_INTERRUPT,
            info    = 0, -- Retail uncleared buffer
            target  =
            {
                {
                    m_uID      = 20000000,
                    result_sum = 1,
                    result     =
                    {
                        {
                            miss          = xi.battle.resolution.HIT,
                            kind          = 0, -- Retail uncleared buffer
                            sub_kind      = 0, -- Retail uncleared buffer
                            info          = 0, -- Retail uncleared buffer
                            scale         = 0,
                            value         = xi.magic.spell.CURE_IV,
                            message       = 0,
                            bit           = 0,
                            has_proc      = false,
                            proc_kind     = 0,
                            proc_info     = 0,
                            proc_value    = 0,
                            proc_message  = 0,
                            has_react     = false,
                            react_kind    = 0,
                            react_info    = 0,
                            react_value   = 0,
                            react_message = 0,
                        },
                    },
                },
            },
        },
    },
    ['Cure Self (Magic Finish)'] =
    {
        ---@param player CClientEntityPair
        test = function(player)
            player:changeJob(xi.job.WHM)
            player:setLevel(99)
            player:addSpell(xi.magic.spell.CURE)
            player.actions:useSpell(player, xi.magic.spell.CURE)
            xi.test.world:skipTime(10) -- Let it complete the cast and tick the AI
        end,
        expected =
        {
            m_uID   = 20000000,
            trg_sum = 1,
            res_sum = 0,
            cmd_no  = xi.battle.category.MAGIC_FINISH,
            cmd_arg = xi.magic.spell.CURE,
            info    = 5, -- Recast
            target  =
            {
                {
                    m_uID = 20000000,
                    result_sum = 1,
                    result =
                    {
                        {
                            miss          = xi.battle.resolution.HIT,
                            kind          = 0,
                            sub_kind      = xi.magic.spell.CURE,
                            info          = 0,
                            scale         = 0,
                            value         = 0,
                            message       = 7,
                            bit           = 0,
                            has_proc      = false,
                            proc_kind     = 0,
                            proc_info     = 0,
                            proc_value    = 0,
                            proc_message  = 0,
                            has_react     = false,
                            react_kind    = 0,
                            react_info    = 0,
                            react_value   = 0,
                            react_message = 0,
                        },
                    },
                },
            },
        },
    },
    ['Stoneskin Self (Magic Finish) (Hit)'] =
    {
        test = function(player)
            player:changeJob(xi.job.RDM)
            player:setLevel(99)
            player:addSpell(xi.magic.spell.STONESKIN)
            player.actions:useSpell(player, xi.magic.spell.STONESKIN)
            xi.test.world:skipTime(10) -- Let it complete the cast and tick the AI
        end,
        expected =
        {
            m_uID   = 20000000,
            trg_sum = 1,
            res_sum = 0,
            cmd_no  = xi.battle.category.MAGIC_FINISH,
            cmd_arg = xi.magic.spell.STONESKIN,
            info    = 26, -- Recast
            target  =
            {
                {
                    m_uID      = 20000000,
                    result_sum = 1,
                    result     =
                    {
                        {
                            miss          = xi.battle.resolution.HIT,
                            kind          = 0,
                            sub_kind      = xi.magic.spell.STONESKIN,
                            info          = 0,
                            scale         = 0,
                            value         = xi.effect.STONESKIN,
                            message       = xi.msg.basic.MAGIC_GAIN_EFFECT,
                            bit           = 0,
                            has_proc      = false,
                            proc_kind     = 0,
                            proc_info     = 0,
                            proc_value    = 0,
                            proc_message  = 0,
                            has_react     = false,
                            react_kind    = 0,
                            react_info    = 0,
                            react_value   = 0,
                            react_message = 0,
                        },
                    },
                },
            },
        },
    },
    ['Stoneskin Self (Magic Finish) (Miss)'] =
    {
        test = function(player)
            player:changeJob(xi.job.RDM)
            player:setLevel(99)
            player:addSpell(xi.magic.spell.STONESKIN)
            player.actions:useSpell(player, xi.magic.spell.STONESKIN)
            xi.test.world:skipTime(10) -- Let it complete the cast and tick the AI
            xi.test.world:skipTime(30) -- Wait for recast
            player.actions:useSpell(player, xi.magic.spell.STONESKIN)
            xi.test.world:skipTime(10) -- Let it complete the cast and tick the AI
        end,
        expected =
        {
            m_uID   = 20000000,
            trg_sum = 1,
            res_sum = 0,
            cmd_no  = xi.battle.category.MAGIC_FINISH,
            cmd_arg = xi.magic.spell.STONESKIN,
            info    = 26, -- Recast
            target  =
            {
                {
                    m_uID = 20000000,
                    result_sum = 1,
                    result =
                    {
                        {
                            miss          = xi.battle.resolution.MISS,
                            kind          = 0,
                            sub_kind      = xi.magic.spell.STONESKIN,
                            info          = 0,
                            scale         = 0,
                            value         = 0,
                            message       = xi.msg.basic.MAGIC_NO_EFFECT,
                            bit           = 0,
                            has_proc      = false,
                            proc_kind     = 0,
                            proc_info     = 0,
                            proc_value    = 0,
                            proc_message  = 0,
                            has_react     = false,
                            react_kind    = 0,
                            react_info    = 0,
                            react_value   = 0,
                            react_message = 0,
                        },
                    },
                },
            },
        },
    },
    ['Composure Self (Ability Finish)'] =
    {
        ---@param player CClientEntityPair
        test = function(player)
            player:changeJob(xi.job.RDM)
            player:setLevel(99)
            player.actions:useAbility(player, xi.jobAbility.COMPOSURE)
        end,
        expected =
        {
            m_uID   = 20000000,
            trg_sum = 1,
            res_sum = 0,
            cmd_no  = xi.battle.category.ABILITY_FINISH,
            cmd_arg = xi.jobAbility.COMPOSURE,
            info    = 0,
            target  =
            {
                {
                    m_uID      = 20000000,
                    result_sum = 1,
                    result     =
                    {
                        {
                            miss          = xi.battle.resolution.HIT,
                            kind          = 2,
                            sub_kind      = 215,
                            info          = 0,
                            scale         = 0,
                            value         = xi.effect.COMPOSURE,
                            message       = xi.msg.basic.USES_JA,
                            bit           = 0,
                            has_proc      = false,
                            proc_kind     = 0,
                            proc_info     = 0,
                            proc_value    = 0,
                            proc_message  = 0,
                            has_react     = false,
                            react_kind    = 0,
                            react_info    = 0,
                            react_value   = 0,
                            react_message = 0,
                        },
                    },
                },
            },
        },
    },
    ['Provoke Mob (Ability Finish)'] =
    {
        ---@param player CClientEntityPair
        test = function(player)
            player:changeJob(xi.job.WAR)
            player:setLevel(99)
            player:gotoZone(xi.zone.WESTERN_ALTEPA_DESERT)
            local spider = player.entities:moveTo(17289267) -- A random spider outside Rabao
            player.actions:useAbility(spider, xi.jobAbility.PROVOKE)
        end,
        expected =
        {
            m_uID   = 20000000,
            trg_sum = 1,
            res_sum = 0,
            cmd_no  = xi.battle.category.ABILITY_FINISH,
            cmd_arg = xi.jobAbility.PROVOKE,
            info    = 0,
            target  =
            {
                {
                    m_uID      = 17289267,
                    result_sum = 1,
                    result     =
                    {
                        {
                            miss          = xi.battle.resolution.HIT,
                            kind          = 2,
                            sub_kind      = 3,
                            info          = 0,
                            scale         = 0,
                            value         = 0,
                            message       = 119,
                            bit           = 0,
                            has_proc      = false,
                            proc_kind     = 0,
                            proc_info     = 0,
                            proc_value    = 0,
                            proc_message  = 0,
                            has_react     = false,
                            react_kind    = 0,
                            react_info    = 0,
                            react_value   = 0,
                            react_message = 0,
                        },
                    },
                },
            },
        },
    },
    ['Ranged Attack Start (Ranged Start)'] =
    {
        ---@param player CClientEntityPair
        test = function(player)
            player:changeJob(xi.job.RNG)
            player:setLevel(99)
            player:addItem(xi.item.MARTIAL_GUN)
            player:addItem(xi.item.SILVER_BULLET)
            player:equipItem(xi.item.MARTIAL_GUN)
            player:equipItem(xi.item.SILVER_BULLET)
            player:gotoZone(xi.zone.WESTERN_ALTEPA_DESERT)
            local spider = player.entities:moveTo(17289267) -- A random spider outside Rabao
            player.actions:rangedAttack(spider)
        end,
        expected =
        {
            m_uID   = 20000000,
            trg_sum = 1,
            res_sum = 0,
            cmd_no  = xi.battle.category.RANGED_START,
            cmd_arg = xi.battle.fourCC.RANGE_START,
            info    = 0,
            target  =
            {
                {
                    m_uID      = 20000000,
                    result_sum = 1,
                    result     =
                    {
                        {
                            miss          = xi.battle.resolution.HIT,
                            kind          = 0,
                            sub_kind      = 55, -- Marksmanship
                            info          = 0,
                            scale         = 0,
                            value         = 0,
                            message       = 0,
                            bit           = 0,
                            has_proc      = false,
                            proc_kind     = 0,
                            proc_info     = 0,
                            proc_value    = 0,
                            proc_message  = 0,
                            has_react     = false,
                            react_kind    = 0,
                            react_info    = 0,
                            react_value   = 0,
                            react_message = 0,
                        },
                    },
                },
            },
        },
    },
    ['Ranged Attack Finish (Ranged Finish)'] =
    {
        ---@param player CClientEntityPair
        test = function(player)
            xi.test.world:setSeed(1)
            -- Force penalty to return 'squarely' message (< 15y)
            _ = stub('xi.combat.ranged.attackDistancePenalty', 14)
            player:changeJob(xi.job.RNG)
            player:setLevel(99)
            player:setMod(xi.mod.RACC, 1000)
            player:addItem(xi.item.POWER_BOW)
            player:addItem(xi.item.WOODEN_ARROW)
            player:equipItem(xi.item.POWER_BOW)
            player:equipItem(xi.item.WOODEN_ARROW)
            player:gotoZone(xi.zone.WESTERN_ALTEPA_DESERT)
            local spider = player.entities:moveTo(17289267) -- A random spider outside Rabao
            player.actions:rangedAttack(spider)
            xi.test.world:skipTime(10)
        end,
        expected =
        {
            m_uID   = 20000000,
            trg_sum = 1,
            res_sum = 0,
            cmd_no  = xi.battle.category.RANGED_FINISH,
            cmd_arg = xi.battle.fourCC.RANGE_FINISH,
            info    = 0,
            target  =
            {
                {
                    m_uID      = 17289267,
                    result_sum = 1,
                    result     =
                    {
                        {
                            miss          = xi.battle.resolution.HIT,
                            kind          = 2,
                            sub_kind      = 0,
                            info          = 0,
                            scale         = 1,
                            value         = 1,
                            message       = xi.msg.basic.RANGED_ATTACK_SQUARELY,
                            bit           = 0,
                            has_proc      = false,
                            proc_kind     = 0,
                            proc_info     = 0,
                            proc_value    = 0,
                            proc_message  = 0,
                            has_react     = false,
                            react_kind    = 0,
                            react_info    = 0,
                            react_value   = 0,
                            react_message = 0,
                        },
                    },
                },
            },
        },
    },
--     ['Ranged Attack Interrupt (Ranged Start)'] =
--     {
--         ---@param player CClientEntityPair
--         test = function(player)
--             player:changeJob(xi.job.RNG)
--             player:setLevel(99)
--             player:addItem(xi.item.POWER_BOW)
--             player:addItem(xi.item.KABURA_ARROW)
--             player:equipItem(xi.item.POWER_BOW)
--             player:equipItem(xi.item.KABURA_ARROW)
--             player:gotoZone(xi.zone.WESTERN_ALTEPA_DESERT)
--             local spider = player.entities:moveTo(17289267) -- A random spider outside Rabao
--             player.actions:rangedAttack(spider)
--             xi.test.world:tickEntity(player)                -- Start shooting
--             player.actions:move(10, 10, 10)
--         end,
--         expected =
--         {
--             m_uID   = 20000000,
--             trg_sum = 1,
--             res_sum = 0,
--             cmd_no  = xi.battle.category.RANGED_START,
--             cmd_arg = xi.battle.fourCC.RANGE_INTERRUPT,
--             info    = 0,
--             target  =
--             {
--                 {
--                     m_uID      = 20000000,
--                     result_sum = 1,
--                     result     =
--                     {
--                         {
--                             miss          = xi.battle.resolution.HIT,
--                             kind          = 0,
--                             sub_kind      = 48,
--                             info          = 0,
--                             scale         = 0,
--                             value         = 0,
--                             message       = 0,
--                             bit           = 0,
--                             has_proc      = false,
--                             proc_kind     = 0,
--                             proc_info     = 0,
--                             proc_value    = 0,
--                             proc_message  = 0,
--                             has_react     = false,
--                             react_kind    = 0,
--                             react_info    = 0,
--                             react_value   = 0,
--                             react_message = 0,
--                         },
--                     },
--                 },
--             },
--         },
--     },
--     ['Ranged Attack Finish (Critical Hit)'] =
--     {
--         ---@param player CClientEntityPair
--         test = function(player)
--             player:changeJob(xi.job.RNG)
--             player:setLevel(99)
--             player:addItem(xi.item.POWER_BOW)
--             player:addItem(xi.item.KABURA_ARROW)
--             player:equipItem(xi.item.POWER_BOW)
--             player:equipItem(xi.item.KABURA_ARROW)
--             player:gotoZone(xi.zone.WESTERN_ALTEPA_DESERT)
--             local spider = player.entities:moveTo(17289267) -- A random spider outside Rabao
--             player.actions:rangedAttack(spider)
--             xi.test.world:skipTime(10)
--         end,
--         expected =
--         {
--             m_uID   = 20000000,
--             trg_sum = 1,
--             res_sum = 0,
--             cmd_no  = xi.battle.category.RANGED_FINISH,
--             cmd_arg = xi.battle.fourCC.RANGE_FINISH,
--             info    = 0,
--             target  =
--             {
--                 {
--                     m_uID      = 17289267,
--                     result_sum = 1,
--                     result     =
--                     {
--                         {
--                             miss          = xi.battle.resolution.HIT,
--                             kind          = 2,
--                             sub_kind      = 0,
--                             info          = 3,
--                             scale         = 3,
--                             value         = 1822,
--                             message       = xi.msg.basic.RANGED_ATTACK_CRIT,
--                             bit           = 0,
--                             has_proc      = false,
--                             proc_kind     = 0,
--                             proc_info     = 0,
--                             proc_value    = 0,
--                             proc_message  = 0,
--                             has_react     = false,
--                             react_kind    = 0,
--                             react_info    = 0,
--                             react_value   = 0,
--                             react_message = 0,
--                         },
--                     },
--                 },
--             },
--         },
--     },
--     ['Ranged Attack Miss (Ranged Finish)'] =
--     {
--         ---@param player CClientEntityPair
--         test = function(player)
--             player:changeJob(xi.job.RNG)
--             player:setLevel(99)
--             player:addItem(xi.item.POWER_BOW)
--             player:addItem(xi.item.KABURA_ARROW)
--             player:equipItem(xi.item.POWER_BOW)
--             player:equipItem(xi.item.KABURA_ARROW)
--             player:gotoZone(xi.zone.WESTERN_ALTEPA_DESERT)
--             local spider = player.entities:moveTo(17289267) -- A random spider outside Rabao
--             player.actions:rangedAttack(spider)
--         end,
--         expected =
--         {
--             m_uID   = 20000000,
--             trg_sum = 1,
--             res_sum = 0,
--             cmd_no  = xi.battle.category.RANGED_FINISH,
--             cmd_arg = xi.battle.fourCC.RANGE_FINISH,
--             info    = 0,
--             target  =
--             {
--                 {
--                     m_uID      = 17289267,
--                     result_sum = 1,
--                     result     =
--                     {
--                         {
--                             miss          = xi.battle.resolution.MISS,
--                             kind          = 2,
--                             sub_kind      = 0,
--                             info          = 0,
--                             scale         = 0,
--                             value         = 0,
--                             message       = xi.msg.basic.RANGED_ATTACK_MISS,
--                             bit           = 0,
--                             has_proc      = false,
--                             proc_kind     = 0,
--                             proc_info     = 0,
--                             proc_value    = 0,
--                             proc_message  = 0,
--                             has_react     = false,
--                             react_kind    = 0,
--                             react_info    = 0,
--                             react_value   = 0,
--                             react_message = 0,
--                         },
--                     },
--                 },
--             },
--         },
--     },
--     ['Bats Sonic Boom (Skill Prepare)'] =
--     {
--         ---@param player CClientEntityPair
--         test = function(player)
--             player:changeJob(xi.job.WAR)
--             player:setLevel(99)
--             player:gotoZone(xi.zone.UPPER_DELKFUTTS_TOWER)
--             local spider = player.entities:moveTo('Incubus Bat')
--             player.actions:useAbility(spider, xi.jobAbility.PROVOKE)
--         end,
--         expected =
--         {
--             m_uID   = 17424516,
--             trg_sum = 1,
--             res_sum = 0,
--             cmd_no  = 7,
--             cmd_arg = 1702125923,
--             info    = 0,
--             target  =
--             {
--                 {
--                     m_uID = 20000000,
--                     result_sum = 1,
--                     result =
--                     {
--                         {
--                             miss          = 0,
--                             kind          = 2,
--                             sub_kind      = 0,
--                             info          = 0,
--                             scale         = 2,
--                             value         = 393,
--                             message       = 43,
--                             bit           = 0,
--                             has_proc      = false,
--                             proc_kind     = 0,
--                             proc_info     = 0,
--                             proc_value    = 0,
--                             proc_message  = 0,
--                             has_react     = false,
--                             react_kind    = 0,
--                             react_info    = 0,
--                             react_value   = 0,
--                             react_message = 0,
--                         },
--                     },
--                 },
--             },
--         },
--     },
--     ['Bats Sonic Boom Miss (Skill Finish)'] =
--     {
--         ---@param player CClientEntityPair
--         test = function(player)
--             player:changeJob(xi.job.WAR)
--             player:setLevel(99)
--             player:gotoZone(xi.zone.WESTERN_ALTEPA_DESERT)
--             local spider = player.entities:moveTo(17289267) -- A random spider outside Rabao
--             player.actions:useAbility(spider, xi.jobAbility.PROVOKE)
--         end,
--         expected =
--         {
--             m_uID   = 17424516,
--             trg_sum = 1,
--             res_sum = 0,
--             cmd_no  = 11,
--             cmd_arg = 393,
--             info    = 0,
--             target  =
--             {
--                 {
--                     m_uID      = 20000000,
--                     result_sum = 1,
--                     result     =
--                     {
--                         {
--                             miss          = 1,
--                             kind          = 3,
--                             sub_kind      = 137,
--                             info          = 0,
--                             scale         = 0,
--                             value         = 0,
--                             message       = 188,
--                             bit           = 0,
--                             has_proc      = false,
--                             proc_kind     = 0,
--                             proc_info     = 0,
--                             proc_value    = 0,
--                             proc_message  = 0,
--                             has_react     = false,
--                             react_kind    = 0,
--                             react_info    = 0,
--                             react_value   = 0,
--                             react_message = 0,
--                         },
--                     },
--                 },
--             },
--         },
--     },
--     ['Ranged with added effect (silence)'] =
--     {
--         ---@param player CClientEntityPair
--         test = function(player)
--             player:changeJob(xi.job.WAR)
--             player:setLevel(99)
--             player:gotoZone(xi.zone.WESTERN_ALTEPA_DESERT)
--             local spider = player.entities:moveTo(17289267) -- A random spider outside Rabao
--             player.actions:useAbility(spider, xi.jobAbility.PROVOKE)
--         end,
--         expected =
--         {
--             m_uID   = 20000000,
--             trg_sum = 1,
--             res_sum = 0,
--             cmd_no  = 2,
--             cmd_arg = 1735157875,
--             info    = 0,
--             target  =
--             {
--                 {
--                     m_uID      = 17424515,
--                     result_sum = 1,
--                     result     =
--                     {
--                         {
--                             miss          = 0,
--                             kind          = 2,
--                             sub_kind      = 0,
--                             info          = 0,
--                             scale         = 2,
--                             value         = 431,
--                             message       = 352,
--                             bit           = 0,
--                             has_proc      = true,
--                             proc_kind     = 13,
--                             proc_info     = 0,
--                             proc_value    = 6,
--                             proc_message  = 160,
--                             has_react     = false,
--                             react_kind    = 0,
--                             react_info    = 0,
--                             react_value   = 0,
--                             react_message = 0,
--                         },
--                     },
--                 },
--             },
--         },
--     },
--     ['Players spikes (Basic Attack)'] =
--     {
--         ---@param player CClientEntityPair
--         test = function(player)
--             player:changeJob(xi.job.WAR)
--             player:setLevel(99)
--             player:gotoZone(xi.zone.WESTERN_ALTEPA_DESERT)
--             local spider = player.entities:moveTo(17289267) -- A random spider outside Rabao
--             player.actions:useAbility(spider, xi.jobAbility.PROVOKE)
--         end,
--         expected =
--         {
--             m_uID   = 17424515,
--             trg_sum = 1,
--             res_sum = 0,
--             cmd_no  = 1,
--             cmd_arg = 812348513,
--             info    = 0,
--             target  =
--             {
--                 {
--                     m_uID      = 20000000,
--                     result_sum = 2,
--                     result     =
--                     {
--                         {
--                             miss          = 0,
--                             kind          = 1,
--                             sub_kind      = 0,
--                             info          = 2,
--                             scale         = 1,
--                             value         = 73,
--                             message       = 67,
--                             bit           = 0,
--                             has_proc      = false,
--                             proc_kind     = 0,
--                             proc_info     = 0,
--                             proc_value    = 0,
--                             proc_message  = 0,
--                             has_react     = true,
--                             react_kind    = 1,
--                             react_info    = 0,
--                             react_value   = 32,
--                             react_message = 44,
--                         },
--                         {
--                             miss          = 1,
--                             kind          = 1,
--                             sub_kind      = 0,
--                             info          = 0,
--                             scale         = 0,
--                             value         = 0,
--                             message       = 15,
--                             bit           = 0,
--                             has_proc      = false,
--                             proc_kind     = 0,
--                             proc_info     = 0,
--                             proc_value    = 0,
--                             proc_message  = 0,
--                             has_react     = false,
--                             react_kind    = 0,
--                             react_info    = 0,
--                             react_value   = 0,
--                             react_message = 0,
--                         },
--                     },
--                 },
--             },
--         },
--     },
--     ['Ridill triple attack (Basic Attack)'] =
--     {
--         ---@param player CClientEntityPair
--         test = function(player)
--             player:changeJob(xi.job.WAR)
--             player:setLevel(99)
--             player:gotoZone(xi.zone.WESTERN_ALTEPA_DESERT)
--             local spider = player.entities:moveTo(17289267) -- A random spider outside Rabao
--             player.actions:useAbility(spider, xi.jobAbility.PROVOKE)
--         end,
--         expected =
--         {
--             m_uID   = 20000000,
--             trg_sum = 1,
--             res_sum = 0,
--             cmd_no  = 1,
--             cmd_arg = 812348513,
--             info    = 0,
--             target  =
--             {
--                 {
--                     m_uID      = 17424515,
--                     result_sum = 3,
--                     result     =
--                     {
--                         {
--                             miss          = 0,
--                             kind          = 1,
--                             sub_kind      = 0,
--                             info          = 0,
--                             scale         = 1,
--                             value         = 91,
--                             message       = 1,
--                             bit           = 0,
--                             has_proc      = false,
--                             proc_kind     = 0,
--                             proc_info     = 0,
--                             proc_value    = 0,
--                             proc_message  = 0,
--                             has_react     = false,
--                             react_kind    = 0,
--                             react_info    = 0,
--                             react_value   = 0,
--                             react_message = 0,
--                         },
--                         {
--                             miss          = 0,
--                             kind          = 1,
--                             sub_kind      = 0,
--                             info          = 0,
--                             scale         = 1,
--                             value         = 84,
--                             message       = 1,
--                             bit           = 0,
--                             has_proc      = false,
--                             proc_kind     = 0,
--                             proc_info     = 0,
--                             proc_value    = 0,
--                             proc_message  = 0,
--                             has_react     = false,
--                             react_kind    = 0,
--                             react_info    = 0,
--                             react_value   = 0,
--                             react_message = 0,
--                         },
--                         {
--                             miss          = 0,
--                             kind          = 1,
--                             sub_kind      = 0,
--                             info          = 0,
--                             scale         = 1,
--                             value         = 100,
--                             message       = 1,
--                             bit           = 0,
--                             has_proc      = false,
--                             proc_kind     = 0,
--                             proc_info     = 0,
--                             proc_value    = 0,
--                             proc_message  = 0,
--                             has_react     = false,
--                             react_kind    = 0,
--                             react_info    = 0,
--                             react_value   = 0,
--                             react_message = 0,
--                         },
--                     },
--                 },
--             },
--         },
--     },
}

-- Credits: atom0s actionparse/bitreader
local function parseBattle2(data)
    local byteOffset = 5 -- Skip first 5 bytes
    local bitOffset  = 0 -- Bit position within current byte
    local action     = {}

    local function readBits(numBits)
        local result   = 0
        local bitsRead = 0

        while bitsRead < numBits do
            -- Get current byte
            local currentByte   = data[byteOffset]

            -- How many bits can we read from current byte?
            local bitsAvailable = 8 - bitOffset
            local bitsToRead    = math.min(numBits - bitsRead, bitsAvailable)

            -- Extract bits using bit library
            local mask          = bit.lshift(1, bitsToRead) - 1
            local bits          = bit.band(bit.rshift(currentByte, bitOffset), mask)

            -- Add to result
            result              = bit.bor(result, bit.lshift(bits, bitsRead))

            -- Update offsets
            bitsRead            = bitsRead + bitsToRead
            bitOffset           = bitOffset + bitsToRead

            -- Move to next byte if needed
            if bitOffset >= 8 then
                bitOffset  = 0
                byteOffset = byteOffset + 1
            end
        end

        return result
    end

    action.m_uID   = readBits(32)
    action.trg_sum = readBits(6)
    action.res_sum = readBits(4)
    action.cmd_no  = readBits(4)
    action.cmd_arg = readBits(32)
    action.info    = readBits(32)
    action.target  = {}
    for _ = 0, action.trg_sum - 1 do
        local target      = {}
        target.m_uID      = readBits(32)
        target.result_sum = readBits(4)
        target.result     = {}

        for _ = 0, target.result_sum - 1 do
            local result    =
            {
                has_proc     = false,
                proc_kind    = 0,
                proc_info    = 0,
                proc_value   = 0,
                proc_message = 0,
            }
            result.miss     = readBits(3)
            result.kind     = readBits(2)
            result.sub_kind = readBits(12)
            result.info     = readBits(5)
            result.scale    = readBits(5)
            result.value    = readBits(17)
            result.message  = readBits(10)
            result.bit      = readBits(31)

            if (readBits(1) > 0) then
                result.has_proc     = true
                result.proc_kind    = readBits(6)
                result.proc_info    = readBits(4)
                result.proc_value   = readBits(17)
                result.proc_message = readBits(10)
            else
                result.has_proc     = false
                result.proc_kind    = 0
                result.proc_info    = 0
                result.proc_value   = 0
                result.proc_message = 0
            end

            if (readBits(1) > 0) then
                result.has_react     = true
                result.react_kind    = readBits(6)
                result.react_info    = readBits(4)
                result.react_value   = readBits(14)
                result.react_message = readBits(10)
            else
                result.has_react     = false
                result.react_kind    = 0
                result.react_info    = 0
                result.react_value   = 0
                result.react_message = 0
            end

            table.insert(target.result, result)
        end

        table.insert(action.target, target)
    end

    return action
end

-- TODO: Replace with some diff library
local function diffPacket(actual, expected, prefix)
    prefix = prefix or 'action'
    local diffs = {}

    for key, expectedValue in pairs(expected) do
        local actualValue = actual[key]
        local fieldName = prefix .. '.' .. key

        if type(expectedValue) == 'table' then
            if type(actualValue) ~= 'table' then
                table.insert(diffs, string.format('%s: expected table, got %s', fieldName, type(actualValue)))
            else
                -- Check size (for arrays)
                if #expectedValue ~= #actualValue then
                    table.insert(diffs, string.format('%s size: %d != %d', fieldName, #actualValue, #expectedValue))
                end

                -- Recursively diff
                local nestedDiffs = diffPacket(actualValue, expectedValue, fieldName)
                for _, diff in ipairs(nestedDiffs) do
                    table.insert(diffs, diff)
                end
            end
        elseif actualValue ~= expectedValue then
            table.insert(diffs, string.format('%s: %s != %s', fieldName, tostring(actualValue), tostring(expectedValue)))
        end
    end

    return diffs
end

describe('BATTLE2 (Action packet)', function()
    ---@type CClientEntityPair
    local player

    before_each(function()
        player = xi.test.world:spawnPlayer()
    end)

    for testName, testDefinition in pairs(packets) do
        it(testName, function()
            testDefinition.test(player)
            xi.test.world:tickEntity(player) -- Tick player AI once to progress states

            -- Retrieve BATTLE2 packets
            local matchedPackets = 0
            local receivedPackets = player.packets:getIncoming()
            for i, packet in pairs(receivedPackets) do
                if packet.type == 0x028 then
                    matchedPackets = matchedPackets + 1
                    local action   = parseBattle2(packet.data)
                    local diffs    = diffPacket(action, testDefinition.expected)
                    if #diffs == 0 then
                        -- Found a matching packet, test successful
                        return
                    end

                    InfoTest(string.format('BATTLE2 candidate #%d, %d diffs', matchedPackets, #diffs))
                    for _, diff in pairs(diffs) do
                        InfoTest(diff)
                    end
                end
            end

            assert(false, 'Could not find any matching BATTLE2 packet')
        end)
    end
end)
