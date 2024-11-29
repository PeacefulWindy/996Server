local _M={}
local class=require "class"
local api=require "api"
local Server=class("TcpServer")

local TcpStatus=
{
    Open=1,
    Close=2,
    Msg=3,
}

function Server:ctor()
    self.ptr=tcpServer.new()
    self.onConnectFunc=nil
    self.onMsgFunc=nil
    api.tcpServers[self.ptr]=self
end

---@param port integer
---@param host string
---@return boolean
function Server:listen(port,host)
    return tcpServer.listen(self.ptr,port,host)
end

---@param fd integer
---@param data string
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
    elseif status == TcpStatus.Close then
        if self.onCloseFunc then
            self.onCloseFunc(self,fd)
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

local Client=class("TcpClient")
function Client:ctor()
    self.ptr=tcpClient.new()
    self.onConnectFunc=nil
    self.onCloseFunc=nil
    self.onMsgFunc=nil
    api.tcpClients[self.ptr]=self
end

function Client:destroy()
    api.tcpClients[self.ptr]=nil
    tcpClient.destroy(self.ptr)
end

---@param host string
---@param port integer
---@return boolean
function Client:connect(host,port)
    return tcpClient.connect(self.ptr,host,port)
end

---@param data string
function Client:send(data)
    tcpClient.send(self.ptr,data,#data)
end

function Client:close()
    tcpClient.close(self.ptr)
end

function Client:onMsg(status,msg)
    if status == TcpStatus.Open then
        if self.onConnectFunc then
            self.onConnectFunc(self)
        end
    elseif status == TcpStatus.Close then
        if self.onCloseFunc then
            self.onCloseFunc(self)
        end
    elseif status == TcpStatus.Msg then
        if self.onMsgFunc then
            self.onMsgFunc(self,msg)
        end
    end
end

function _M.newServer()
    return Server.new()
end

function _M.newClient()
    return Client.new()
end

return _M