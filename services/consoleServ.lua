local api=require "api"
local tcp=require "net.tcp"
local sysArgs=api.args()

local host=sysArgs.host or "127.0.0.1"
local port=sysArgs.port
assert(port,"console service need port args!")

local tcpServer=tcp.newServer()

local CMD={}

function CMD.help(fd)
    local helps=
    {
        "---- commands -----",
        "serviceName cmd args...:call service",
        "close:close server",
        "exit:exit console.",
        "-------------------",
    }
    
    tcpServer:send(fd,table.concat(helps,""))
end

function CMD.close()
    api.exit()
end

function CMD.exit(fd)
    tcpServer:send(fd,"bye!")
    tcpServer:close(fd)
end

tcpServer.onConnectFunc=function(_,fd)
    tcpServer:send(fd,"---Welcome to 996Server console---")
end

tcpServer.onMsgFunc=function(_,fd,msg)
    if #msg == 0 then
        return
    end
    
    local cmds=api.split(msg," ")
    local serviceName=cmds[1]
    table.remove(cmds,1)
    local func=CMD[serviceName]
    if func then
        func(fd,cmds)
        return
    end

    local serviceId=api.queryService(serviceName)
    if serviceId <= 0 then
        tcpServer:send(fd,"invalid service Name")
        return
    end

    local isOk,ret=api.call(serviceId,table.unpack(cmds))
    if not isOk then
        tcpServer:send(fd,"command error!")
        return
    end

    tcpServer:send(fd,"command ok!")
end

local tcpClient=tcp.newClient()
tcpClient.onMsgFunc=function(_,msg)
    api.info(msg)
end

local _P={}
local cmds={}
function _P.loopEvent()
    local ch=core.getInputChar()
    if ch then
        if ch == 13 then
            for k,v in pairs(cmds)do
                cmds[k]=string.char(v)
            end
            tcpClient:send(table.concat(cmds,""))
            cmds={}
        elseif ch == "\b" then
            table.remove(cmds,#cmds)
            core.print("\b")
        else
            table.insert(cmds,ch)
        end
    end

    api.async(_P.loopEvent)
end

api.async(function()
    assert(tcpServer:listen(port,host),string.format("listen %s:%d failed!",host,port))
    assert(tcpClient:connect(host,port),string.format("connect %s:%d failed!",host,port))

    api.async(_P.loopEvent)
end)

api.shutdown(function()
    if tcpServer then
        tcpServer:destroy()
    end

    if tcpClient then
        tcpClient:destroy()
    end
end)