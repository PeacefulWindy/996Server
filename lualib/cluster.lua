local _M={}
local api=require "api"

local clusterId

---@param nodeId integer
---@param serviceName string
function _M.send(nodeId,serviceName,...)
    if not clusterId then
        clusterId=api.queryService("cluster")
    end
    assert(clusterId > 0,"not found cluster service!")

    api.send(clusterId,nodeId,"send",serviceName,SERVICE_ID,...)
end

---@param nodeId integer
---@param serviceName string
function _M.call(nodeId,serviceName,...)
    if not clusterId then
        clusterId=api.queryService("cluster")
    end
    assert(clusterId > 0,"not found cluster service!")

    return api.call(clusterId,"send",nodeId,serviceName,SERVICE_ID,...)
end

return _M