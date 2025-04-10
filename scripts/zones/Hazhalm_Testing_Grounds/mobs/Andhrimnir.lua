-----------------------------------
-- Area: Hazhalm Testing Grounds
--   NM: Andhrimnir (Einherjar)
-----------------------------------
mixins =
{
    require('scripts/mixins/draw_in'),
}
-----------------------------------
-- Regular Corse TP moves + Final Retribution
-- Resists Bind/Blind/Gravity
-- Teleports to a random party member and charms them with Danse Macabre
-- No standback
---@type TMobEntity
local entity = {}

local function vanish(mob)
    mob:setAutoAttackEnabled(false)
    mob:setMagicCastingEnabled(false)
    mob:setMobAbilityEnabled(false)
    mob:setStatus(xi.status.INVISIBLE)
    mob:hideName(true)
    mob:setUntargetable(true)
end

local function reset(mob)
    mob:hideName(false)
    mob:setUntargetable(false)
    mob:setStatus(xi.status.UPDATE)
    mob:setAutoAttackEnabled(true)
    mob:setMagicCastingEnabled(true)
    mob:setMobAbilityEnabled(true)
    mob:setLocalVar('victimId', 0)
    mob:setLocalVar('nextCharm', os.time() + math.random(50,70))
end

entity.onMobInitialize = function(mob)
    xi.einherjar.onBossInitialize(mob)
    mob:addImmunity(xi.immunity.BIND)
    mob:addImmunity(xi.immunity.BLIND)
    mob:addImmunity(xi.immunity.GRAVITY)

    -- Silence is unconfirmed
    mob:addImmunity(xi.immunity.SILENCE)

    mob:setMobMod(xi.mobMod.NO_STANDBACK, 1)
end

-- Disappear, then reappear next to a random target, then use Dance Macabre
entity.onMobFight = function(mob, target)
    -- Don't process if the mob is busy
    if mob:getCurrentAction() ~= xi.act.ATTACK then
        return
    end

    if  mob:getLocalVar('victimId') ~= 0 then
        if mob:getLocalVar('reappearAt') <= os.time() then
            return
        end

        local victim = GetPlayerByID(mob:getLocalVar('victimId'))
        if not victim or not victim:isAlive() then
            reset(mob)
            return
        end

        mob:setPos(victim:getXPos() - 1, victim:getYPos() + 3, victim:getZPos(), 0)
        mob:facePlayer(victim)

        reset(mob)
        mob:useMobAbility(533) -- danse_macabre
        return
    end

    if mob:getLocalVar('nextCharm') < os.time() then
        local enmityList = mob:getEnmityList()
        local randomEnmityEntity
        if #enmityList == 0 then
            randomEnmityEntity = target
        else
            randomEnmityEntity = utils.randomEntry(enmityList).entity
        end

        mob:setLocalVar('victimId', randomEnmityEntity:getID())
        mob:setLocalVar('reappearAt', os.time() + 3)
        vanish(mob)
    end
end

entity.onMobDisengage = function(mob)
    reset(mob)
end

entity.onMobEngage = function(mob, target)
    reset(mob)
end

return entity
