local class=require "class"
local _M=class("proto")

function _M:ctor()
    self.ptr=proto.new()
end

function _M:destroy()
    proto.destroy(self.ptr)
end

function _M:loadFile(filePath)
    return proto.loadFile(self.ptr,filePath)
end

function _M:decode(msgType,data)
    return proto.decode(self.ptr,msgType,data)
end

function _M:encode(msgType,data)
    return proto.encode(self.ptr,msgType,data)
end

return _M