---@meta

-- luacheck: ignore 241
---@class CTriggerArea
local CTriggerArea = {}

---@nodiscard
---@return integer
function CTriggerArea:getTriggerAreaID()
end

---@nodiscard
---@return integer
function CTriggerArea:getCount()
end

---@param count integer
---@return integer
function CTriggerArea:addCount(count)
end

---@param count integer
---@return integer
function CTriggerArea:delCount(count)
end
