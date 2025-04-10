-----------------------------------
-- Area: Hazhalm Testing Grounds
--   NM: Balrahn (Einherjar)
-----------------------------------
mixins =
{
    require('scripts/mixins/draw_in'),
}
-----------------------------------
-- Use extended set of Soulflayer moves
-- Starts by just attacking with 180 delay
-- Does not cast magic until using Immortal Shield
-- At that point, it will cast one of 3 -ga spells every 15 seconds until death
-- Its delay is also increased in that mode to 595-600
---@type TMobEntity
local entity = {}

entity.onMobInitialize = function(mob)
    xi.einherjar.onBossInitialize(mob)
    mob:addImmunity(xi.immunity.BLIND)
    mob:addImmunity(xi.immunity.SILENCE)

    mob:setMobMod(xi.mobMod.NO_STANDBACK, 1)
end

entity.onMobSpawn = function(mob)
    mob:setMagicCastingEnabled(false)
    mob:setMobMod(xi.mobMod.MAGIC_DELAY, 0)
    mob:setMobMod(xi.mobMod.MAGIC_COOL, 15)
end

entity.onMobWeaponSkill = function(target, mob, skill)
    local skillId = skill:getID()

    if
        skillId == 1965 -- immortal_shield
    then
        mob:setMagicCastingEnabled(true)
        mob:setDelay(10 * 1000)
    end
end

return entity
