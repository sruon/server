-----------------------------------
-- Area: Al'Taieu
--  NPC: qm_jailer_of_prudence (???)
-- Allows players to spawn the Jailer of Prudence by trading the Third Virtue, Deed of Sensibility, and High-Quality Hpemde Organ to a ???.
-- !pos , 706 -1 22
-----------------------------------
local ID = zones[xi.zone.ALTAIEU]
-----------------------------------
---@type TNpcEntity
local entity = {}

entity.declaredTrades =
{
    {
        match   = {
            items = {
                { xi.item.THIRD_VIRTUE,               1 },
                { xi.item.DEED_OF_SENSIBILITY,        1 },
                { xi.item.HIGH_QUALITY_HPEMDE_ORGAN, 1 },
            },
        },
        onSuccess = function(player, npc)
            return npcUtil.popFromQM(player, npc,
                { ID.mob.JAILER_OF_PRUDENCE, ID.mob.JAILER_OF_PRUDENCE + 1 })
        end,
    },
}

entity.onTrigger = function(player, npc)
    player:startEvent(202)
end

return entity
