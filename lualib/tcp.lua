local _M={}
local class=require "class"
local api=require "api"
local Server=class("TcpServer")

local TcpStatus=
{
    Open=1,
    Close=2,
    Msg=2,
}

function Server:ctor(...)
    self.ptr=tcpServer.new()
    self.onConnectFunc=nil
    self.onMsgFunc=nil
    api.tcpServers[self.ptr]=self
end

function Server:listen(port,host)
    return tcpServer.listen(self.ptr,port,host)
end

function Server:send(fd,data)
    if #data == 0 then
        return
    end

    tcpServer.send(self.ptr,fd,data,#data)
end

function Server:onMsg(fd,status,msg)
    if status == TcpStatus.Open then
        if self.onConnectFunc then
            self.onConnectFunc(self,fd)
        end
    elseif status == TcpStatus.Msg then
        if self.onMsgFunc then
            self.onMsgFunc(self,fd,msg)
        end
    end
end

function Server:destroy()
    api.tcpServers[self.ptr]=nil
    tcpServer.destroy(self.ptr)
end

function _M.newServer()
    return Server.new()
end

return _M