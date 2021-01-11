-----------------------------------
-- Area: Caedarva Mire
--  NPC: Warhorse Hoofprint
-----------------------------------
require("scripts/globals/dark_rider")

function onTrigger(player, npc)
    return darkRider.hoofprintTrigger(player, npc)
end
