-----------------------------------
-- Area: Arrapago Reef
--   NM: Bukki
-----------------------------------
mixins =
{
    require("scripts/mixins/job_special"),
    require("scripts/mixins/families/imp"),
}
-----------------------------------
---@type TMobEntity
local entity = {}

entity.onMobSpawn = function(mob)
    dsp.mix.jobSpecial.config(mob, {
        hpp = math.random(10, 50),
    })
    mob:setTP(3000)
    mob:SetMobSkillAttack(0)
    mob:SetMagicCastingEnabled(true)
    mob:setMobMod(dsp.mobMod.IDLE_DESPAWN, 300)
    mob:setMobMod(dsp.mobMod.GIL_MAX, -1)
    mob:setLocalVar("tp_spam", math.random(5, 15))
    mob:addListener("COMBAT_TICK", "BUKKI_TICK", function(mob)
        if mob:getHPP() <= mob:getLocalVar("tp_spam") and mob:getLocalVar("tp_spam") > 0 then
            mob:setLocalVar("tp_spam", 0)
            mob:SetMagicCastingEnabled(false)
            mob:SetMobSkillAttack(3002)
        end
    end)
end

entity.onMobDeath = function(mob, player, optParams)
end

return entity
