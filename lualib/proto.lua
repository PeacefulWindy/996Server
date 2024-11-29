local class=require "class"
local _M=class("proto")

function _M:ctor()
    self.ptr=proto.new()
end

function _M:destroy()
    proto.destroy(self.ptr)
end

---@param filePath string
---@return boolean
function _M:loadFile(filePath)
    return proto.loadFile(self.ptr,filePath)
end

---@param msgType string @ proto message type
---@param data string @ proto data
---@return boolean,table?
function _M:decode(msgType,data)
    return proto.decode(self.ptr,msgType,data)
end

---@param msgType string @ proto message type
---@param data table @ proto data
---@return boolean,string?
function _M:encode(msgType,data)
    return proto.encode(self.ptr,msgType,data)
end

return _M