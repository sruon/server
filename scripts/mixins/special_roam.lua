-----------------------------------
-- Special Roam Mixin
-- For mobs that change animation state between idle/combat (Phuabo, Yovra, Hpemde, Amphiptere, etc.)
-----------------------------------

---@class SpecialRoamOptions
---@field idleAnim integer AnimationSub when idle/hidden on spawn (default: 5)
---@field fightAnim integer AnimationSub when fighting (default: 6)
---@field returnAnim integer? AnimationSub when returning to idle after disengage (default: idleAnim)
---@field returnDelay integer Seconds before returning to idle after disengage, 0 = immediate (default: 0)

---@param options SpecialRoamOptions?
---@return fun(mob: CBaseEntity)
local function specialRoam(options)
    options           = options or {}
    local idleAnim    = options.idleAnim or 5
    local fightAnim   = options.fightAnim or 6
    local returnAnim  = options.returnAnim or idleAnim
    local returnDelay = options.returnDelay or 0

    return function(mob)
        mob:addListener('SPAWN', 'SPECIAL_ROAM_SPAWN', function(m)
            m:hideName(true)
            m:setUntargetable(true)
            m:setCollisionDisabled(true)
            m:setAnimationSub(idleAnim)
            m:wait(2000)
        end)

        mob:addListener('ENGAGE', 'SPECIAL_ROAM_ENGAGE', function(m, target)
            m:setLocalVar('specialRoamReturnActive', 0)
            m:hideName(false)
            m:setUntargetable(false)
            m:setCollisionDisabled(false)
            m:setAnimationSub(fightAnim)
            m:wait(2000)
        end)

        mob:addListener('DISENGAGE', 'SPECIAL_ROAM_DISENGAGE', function(m)
            if returnDelay > 0 then
                m:setLocalVar('specialRoamReturnActive', 1)
                m:timer(returnDelay * 1000, function(mArg)
                    if
                        mArg and
                        mArg:getLocalVar('specialRoamReturnActive') == 1 and
                        not mArg:isEngaged()
                    then
                        mArg:hideName(true)
                        mArg:setUntargetable(true)
                        mArg:setCollisionDisabled(true)
                        mArg:setAnimationSub(returnAnim)
                    end
                end)
            else
                m:hideName(true)
                m:setUntargetable(true)
                m:setCollisionDisabled(true)
                m:setAnimationSub(returnAnim)
                m:wait(2000)
            end
        end)
    end
end

return specialRoam
