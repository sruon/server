-----------------------------------
-- Fishing
-- The map server hands every fishing request here and keeps no fishing state of its own.
-- Data: data/fishing.yaml and data/zones/<zone>/fishing_areas.yaml.
-----------------------------------
xi = xi or {}
xi.fishing = xi.fishing or {}

---@return TFishingData
xi.fishing.getData = function()
    if not xi.fishing.data then
        xi.fishing.data = GetFishingData()
    end

    return xi.fishing.data
end

-- Modes of the 0x110 packet the client sends while a cast is in progress.
xi.fishing.mode =
{
    CHECK_HOOK        = 2,
    END_MINIGAME      = 3,
    RELEASE           = 4,
    POTENTIAL_TIMEOUT = 5,
}

-- The player cast their line (action 14).
-- Return the hook timer in seconds to start the cast. Return nothing to refuse it; the core then
-- ends the fishing event on the client.
--
-- Example:
--   xi.fishing.onStart = function(player)
--       if player:getEquipID(xi.slot.RANGED) == 0 then
--           -- "You can't fish without a rod in your hands." sits one past the zone's offset
--           player:messageSpecial(zones[player:getZoneID()].text.FISHING_MESSAGE_OFFSET + 1)
--           return nil
--       end
--
--       -- the fish ranking contest and GetRecentFishers read this var
--       player:setCharVar('[Fish]LastCastTime', GetSystemTime())
--       player:setAnimation(xi.animation.NEW_FISHING_START)
--       return 13
--   end
---@param player CBaseEntity
---@return integer?
xi.fishing.onStart = function(player)
    return nil
end

-- CHECK_HOOK: the client's hook timer ran out and it asks whether anything bit. Return the fight
-- for the client to run and the core sends it as the 0x115 packet. Keys are the packet's own:
-- stamina, regen, move_frequency, arrow_damage, arrow_delay, arrow_regen, time, angler_sense,
-- intuition. Return nothing for an empty cast.
--
-- Everything the bite roll reads is already reachable:
--   local data    = xi.fishing.getData()
--   local zone    = data.zones[player:getZoneID()]
--   local rod     = data.rods[player:getEquipID(xi.slot.RANGED)]
--   local bait    = data.baits[player:getEquipID(xi.slot.AMMO)]
--   local skill   = player:getSkillLevel(xi.skill.FISHING) / 10 + player:getMod(xi.mod.FISH)
--   local moon    = VanadielMoonPhase()   -- 0 to 100, new at 0 and full at 100
--   local hour    = VanadielHour()
--   local weather = player:getWeather()
--   local rumors  = player:hasKeyItem(xi.ki.SERPENT_RUMORS)
--   local belly   = player:getQuestStatus(xi.questLog.OTHER_AREAS, xi.quest.id.otherAreas.INSIDE_THE_BELLY)
--
-- Finding the area the player stands in, shaped areas first and the whole-zone area as the fallback:
--   local function inArea(area)
--       if area.cylinder then
--           return player:isInsideCylinder(area.cylinder.x, area.cylinder.z, area.cylinder.radius)
--       elseif area.poly then
--           return player:isInsidePoly(area.poly)
--       end
--
--       return true
--   end
--
-- Quests get the first say. A quest section declares onFishingHook at zone level and returns the
-- catch to force, an item id or a mob entity, while its check holds; nothing means roll as usual.
--   local forced = InteractionGlobal.onFishingHook(player, areaName)
--   if type(forced) == 'number' then
--       -- forced is the item id to land, Hydrogauge for the COR AF2 quest
--   elseif forced then
--       -- forced is the monster to hook, Odontotyrannus for A Boy's Dream
--   end
--
-- In the quest file:
--   [xi.zone.ARRAPAGO_REEF] =
--   {
--       onFishingHook = function(player, area)
--           if quest:getVar(player, 'Prog') == 0 then
--               return xi.item.HYDROGAUGE
--           end
--       end,
--   },
--
-- A monster is only a candidate while its spawn is dead and not already on someone's line:
--   local mob = GetMobByID(17396141)
--   if mob and not mob:isAlive() and mob:getLocalVar('hooked') == 0 then
--       mob:setLocalVar('hooked', 1)
--   end
--
-- Example, a moat carp on the hook:
--   player:setAnimation(xi.animation.NEW_FISHING_FISH)
--   return
--   {
--       stamina        = 23 * math.random(95, 105),
--       regen          = 128,
--       move_frequency = 100,
--       arrow_damage   = 340,
--       arrow_delay    = 10,
--       arrow_regen    = 70,
--       time           = 30,
--       angler_sense   = 1,
--       intuition      = 30,
--   }
---@param player CBaseEntity
---@return table?
local function onCheckHook(player)
end

-- END_MINIGAME: the fight is over. para is the fish's remaining stamina as the client reports it:
--   0 to 4    the client claims the catch; roll the loss, line snap and rod break, never trust it
--   5 to 20   lost to lack of skill
--   21 to 100 the line broke
--   200       the player gave up
--   300       the catch got away
-- para2 echoes the intuition value the fight was sent with and must match it.
--
-- Landing a large fish: the item takes its size and weight as fish exdata, then the 0x027 catch
-- message goes to everyone in range with the item, the weight and the stamina term.
--   local ID   = zones[player:getZoneID()]
--   local fish = player:addItem({ id = xi.item.LIK, quantity = 1 })
--   if fish then
--       fish:setExData({ size = 320, weight = 1520, isRanked = false })
--   end
--
--   player:setAnimation(xi.animation.NEW_FISHING_CAUGHT)
--   player:messageSpecial(ID.text.FISHING_MESSAGE_OFFSET + 0x27, xi.item.LIK, 1520, 5, math.floor(stamina * 0.12))
--
-- Landing a monster: put it behind the player, bring it up and hand it the claim.
--   local pos = player:getPos()
--   local mob = GetMobByID(17396141)
--   mob:setPos(pos.x, pos.y, pos.z, pos.rot)
--   mob:spawn()
--   mob:updateClaim(player)
--
-- Landing a chest: the Jade Etui NPC is shown beside the player and the chart quest takes over.
--   local chest = GetNPCByID(17261035)
--   chest:setPos(pos.x, pos.y, pos.z, pos.rot)
--   chest:setStatus(xi.status.NORMAL)
--
-- Bait is consumed on every hook, a lure only on a line break:
--   player:delItem(baitId, 1)
--
-- A broken rod is swapped for its broken counterpart from the catalog's breaks_to.
---@param player CBaseEntity
---@param para integer
---@param para2 integer
local function onEndMiniGame(player, para, para2)
end

-- RELEASE: the client has shown the result and the cast is over. Follows every outcome, empty
-- casts included. Unhook the monster, roll skill-ups, clear the animation, drop the state.
--
-- A skill-up writes the new value and tells the player. Skill is stored in tenths.
--   local skill = player:getSkillLevel(xi.skill.FISHING)
--   player:setSkillLevel(xi.skill.FISHING, skill + 2)
--   player:messageBasic(38, xi.skill.FISHING, 2) -- <skill> skill rises 0.2 points
--   if math.floor((skill + 2) / 10) > math.floor(skill / 10) then
--       player:messageBasic(xi.msg.basic.SKILL_REACHES_LEVEL, xi.skill.FISHING, math.floor((skill + 2) / 10))
--   end
--
-- The daily counters live in char vars that expire at Japan midnight:
--   local caught = player:getCharVar('[Fish]DailyCatches') + 1
--   player:setCharVar('[Fish]DailyCatches', caught, JstMidnight())
--
-- Let the monster go and end the animation:
--   mob:setLocalVar('hooked', 0)
--   player:setAnimation(xi.animation.NONE)
---@param player CBaseEntity
local function onRelease(player)
end

-- POTENTIAL_TIMEOUT: the client warns the fight is about to time out. para is the seconds left.
-- "You don't know how much longer you can keep this one on the line..."
---@param player CBaseEntity
---@param para integer
local function onPotentialTimeout(player, para)
end

-- The client advanced the cast (0x110).
---@param player CBaseEntity
---@param mode integer one of xi.fishing.mode
---@param para integer
---@param para2 integer
---@return table? the fight to send, on CHECK_HOOK only
xi.fishing.onAction = function(player, mode, para, para2)
    return switch(mode): caseof
    {
        [xi.fishing.mode.CHECK_HOOK] = function()
            return onCheckHook(player)
        end,

        [xi.fishing.mode.END_MINIGAME] = function()
            onEndMiniGame(player, para, para2)
        end,

        [xi.fishing.mode.RELEASE] = function()
            onRelease(player)
        end,

        [xi.fishing.mode.POTENTIAL_TIMEOUT] = function()
            onPotentialTimeout(player, para)
        end,
    }
end

-- A hostile action or a zone line interrupted the cast.
---@param player CBaseEntity
xi.fishing.onInterrupt = function(player)
end
