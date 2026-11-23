-----------------------------------
-- Area: Balgas Dais
-- Name: Mission Rank 2
-- !pos 299 -123 345 146
-----------------------------------

local content = Battlefield:new({
    battlefieldId = xi.battlefield.id.RANK_2_MISSION,
})

function content:entryRequirement(player, npc, isRegistrant, trade)
    local isCurrentMission    = player:getCurrentMission(xi.mission.log_id.SANDORIA) == xi.mission.id.sandoria.JOURNEY_TO_WINDURST2 or
        player:getCurrentMission(xi.mission.log_id.BASTOK) == xi.mission.id.bastok.THE_EMISSARY_WINDURST2
    local currentRequirements = isCurrentMission and player:hasKeyItem(xi.keyItem.DARK_KEY)
    local nonRegistrantReqs   = player:hasCompletedMission(player:getNation(), 5) or currentRequirements

    return (not isRegistrant and nonRegistrantReqs) or currentRequirements
end

function content:checkSkipCutscene(player)
    return player:hasCompletedMission(xi.mission.log_id.SANDORIA, xi.mission.id.sandoria.JOURNEY_TO_WINDURST2) or
        player:hasCompletedMission(xi.mission.log_id.BASTOK, xi.mission.id.bastok.THE_EMISSARY_WINDURST2) or
        (player:getMissionStatus(player:getNation()) > 8 and
        (
            player:getCurrentMission(xi.mission.log_id.SANDORIA) == xi.mission.id.sandoria.JOURNEY_TO_WINDURST2 or
            player:getCurrentMission(xi.mission.log_id.BASTOK) == xi.mission.id.bastok.THE_EMISSARY_WINDURST2
        ))
end

content:addEssentialMobs({ 'Black_Dragon', 'Searcher' })

return content:register()
