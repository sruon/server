-----------------------------------
-- Flames for the Dead
-- Bearclaw Pinnacle mission battlefield
-- !pos -720 9 -441 6
-----------------------------------
local ID = zones[xi.zone.BEARCLAW_PINNACLE]
-----------------------------------

local content = BattlefieldMission:new({
    battlefieldId         = xi.battlefield.id.FLAMES_FOR_THE_DEAD,

    missionArea           = xi.mission.log_id.COP,
    mission               = xi.mission.id.cop.THREE_PATHS,
    missionStatusArea     = xi.mission.log_id.COP,
    missionStatus         = xi.mission.status.COP.ULMIA,
    requiredMissionStatus = 8,
    title                 = xi.title.TRUE_COMPANION_OF_LOUVERANCE,
})

content:addEssentialMobs({ 'Snoll_Tzar' })

function content:onBattlefieldLoss(player, battlefield)
    player:messageSpecial(ID.text.BLOWN_AWAY)

    Battlefield.onBattlefieldLoss(self, player, battlefield)
end

return content:register()
