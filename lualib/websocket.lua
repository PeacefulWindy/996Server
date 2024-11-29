local _M={}
local class=require "class"
local api=require "api"
local Server=class("WebsocketServer")

local WebsocketStatus=
{
    Open=1,
    Close=2,
    Msg=3,
}

function Server:ctor()
    self.ptr=websocketServer.new()
    self.onConnectFunc=nil
    self.onCloseFunc=nil
    self.onMsgFunc=nil
    api.websocketServers[self.ptr]=self
end

---@param port integer
---@param host string
---@param maxConnection integer
---@return boolean
function Server:listen(port,host,maxConnection)
    return websocketServer.listen(self.ptr,port,host,maxConnection)
end

---@param fd integer
---@param data string
function Server:send(fd,data)
    if #data == 0 then
        return
    end

    websocketServer.send(self.ptr,fd,data)
end

function Server:onMsg(fd,status,msg)
    if status == WebsocketStatus.Open then
        if self.onConnectFunc then
            self.onConnectFunc(self,fd)
        end
    elseif status == WebsocketStatus.Close then
        if self.onCloseFunc then
            self.onCloseFunc(self,fd)
        end
    elseif status == WebsocketStatus.Msg then
        if self.onMsgFunc then
            self.onMsgFunc(self,fd,msg)
        end
    end
end

function Server:destroy()
    api.websocketServers[self.ptr]=nil
    websocketServer.destroy(self.ptr)
end

function _M.newServer()
    return Server.new()
end

return _M