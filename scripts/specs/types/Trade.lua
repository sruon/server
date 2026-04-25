---@meta

-----------------------------------
-- Declarative NPC trade API.
--
-- Scripts list what trades an NPC accepts via entity.declaredTrades.
-- The engine matches, reserves, dispatches events, commits, rolls back.
--
--     entity.declaredTrades =
--     {
--         {
--             match = { items = { { xi.item.IRON_INGOT, 3 }, { xi.item.LEATHER, 2 } } },
--             event =
--             {
--                 id = 1234,
--                 onFinish = function(player, option, npc, confirmations)
--                     return npcUtil.giveItem(player, xi.item.MYTHRIL_SWORD)
--                 end,
--             },
--         },
--     }
--
-- Per decl: acceptIf (optional predicate) → reserve items → either
-- sync onSuccess, or start event.id and run onFinish at event end.
-- First passing decl wins.
-----------------------------------

---@class TTradeOffer
---@field id   xi.item
---@field qty  integer
---@field item CLuaItem?

---@alias TTradeItemEntry { [1]: xi.item, [2]: integer? }

---@alias TTradeItemsFn   fun(player: CBaseEntity, npc: CBaseEntity, offers: TTradeOffer[]): TTradeItemEntry[]
---@alias TTradeGilFn     fun(player: CBaseEntity, npc: CBaseEntity, offers: TTradeOffer[]): integer
---@alias TTradeIdFn      fun(player: CBaseEntity, npc: CBaseEntity, offers: TTradeOffer[]): integer
---@alias TTradeParamsFn  fun(player: CBaseEntity, npc: CBaseEntity, offers: TTradeOffer[]): integer[]

---@class TTradeMatch
---@field items?       TTradeItemEntry[]|TTradeItemsFn
---@field anyOf?       xi.item[]
---@field allowExtras? boolean
---@field gil?         integer|TTradeGilFn

---@alias TTradeConfirmations table<xi.item, integer>

---@class TTradeFinishAction
---@field commit?       boolean
---@field confirmItems? TTradeItemEntry[]

---@class TTradeEvent
---@field id        integer|TTradeIdFn
---@field params?   integer[]|TTradeParamsFn
---@field onFinish? fun(player: CBaseEntity, option: integer, npc: CBaseEntity, confirmations: TTradeConfirmations): boolean|TTradeFinishAction|nil

---@class TDeclaredTrade
---@field match?     TTradeMatch
---@field acceptIf?  fun(player: CBaseEntity, npc: CBaseEntity, offers: TTradeOffer[]): boolean
---@field onSuccess? fun(player: CBaseEntity, npc: CBaseEntity): boolean|nil
---@field event?     TTradeEvent
