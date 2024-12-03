local api=require "api"
local http=require "net.http"
local tcp=require "net.tcp"
local SysArgs=api.args()
local _P={}

local tcpServer
local tcpClients={}

function _P.onConnect(_,fd)
    local remoteInfo=tcpServer:getRemoteInfo(fd)
    api.info(string.format("fd:[%d] %s:%d connect",fd,remoteInfo.host,remoteInfo.port))
end

function _P.sendResponse(fd,toServId,session,isOk,msg)
    if isOk then
        isOk=1
    else
        isOk=0
    end
    local dataLen=#msg
    local respPackData=string.pack(string.format("IIIIc%d",dataLen),dataLen,toServId,session,isOk,msg)
    tcpServer:send(fd,respPackData)
end

function _P.onMsg(_,fd,msg)
    local reqDataLen,nextIndex=string.unpack("I",msg)
    local toServiceName,fromServiceId,session,reqData=string.unpack(string.format("zIIc%d",reqDataLen),msg,nextIndex)
    reqData=json.decode(reqData)

    local serviceId=api.queryService(toServiceName)
    if serviceId <= 0 then
        local errMsg=string.format("not found service id:%s",toServiceName)
        api.error(errMsg)
        if session > 0 then
            _P.sendResponse(fd,fromServiceId,session,false,errMsg)
        end
    end

    if session == 0 then
        api.send(api.MsgType.Lua,serviceId,table.unpack(reqData))
    else
        api.async(function()
            local ret=table.pack(api.call(api.MsgType.Lua,serviceId,table.unpack(reqData)))
            local isOk=ret[1]
            table.remove(ret,1)
            ret["n"]=nil
            local respData=json.encode(ret)
            _P.sendResponse(fd,fromServiceId,session,isOk,respData)
        end)
    end
end

function _P.onClose(_,fd)
    local remoteInfo=tcpServer:getRemoteInfo(fd)
    api.info(string.format("fd:[%d] %s:%d disconnect",fd,remoteInfo.host,remoteInfo.port))
end

function _P.onClientResponse(_,msg)
    local reqDataLen,nextIndex=string.unpack("I",msg)
    local toServiceId,session,isOk,respData=string.unpack(string.format("IIIc%d",reqDataLen),msg,nextIndex)
    respData=json.decode(respData)
    if isOk == 0 then
        api.response(toServiceId,session,false,table.unpack(respData))
    else
        api.response(toServiceId,session,true,table.unpack(respData))
    end
end

function _P.onDispatch(source,session,toNodeId,toServiceName,fromServiceId,...)
    local args={...}
    local data=json.encode(args)
    local dataLen=#data
    local packData=string.pack(string.format("IzIIc%d",dataLen),dataLen,toServiceName,fromServiceId,session,data)

    api.async(function()
        local tcpClient=tcpClients[toNodeId]
        if not tcpClient then
            local response=http.get(string.format(SysArgs.url,toNodeId))
            assert(response.status==200,"request failed!")

            local data=json.decode(response.body)
            local host=data.host
            local port=data.port

            tcpClient=tcp.newClient()
            tcpClient.onMsgFunc=_P.onClientResponse

            if not tcpClient:connect(host,port) then
                api.error(string.format("connect node %d failed!",toNodeId))
                return
            end

            tcpClients[toNodeId]=tcpClient

            api.info(string.format("node[%d] %s:%d connect",toNodeId,host,port))
        end

        tcpClient:send(packData)
    end)
end

api.async(function()
    local nodeId=api.env("node")
    assert(nodeId,"invalid node env!")

    assert(SysArgs.url,"invalid url!")
    local url=string.format(SysArgs.url,nodeId)
    local response=http.get(url)
    assert(response.status==200,string.format("%s request failed!",url))

    local data=json.decode(response.body)
    local host=data.host
    local port=data.port

    tcpServer=tcp.newServer()
    tcpServer.onConnectFunc=_P.onConnect
    tcpServer.onMsgFunc=_P.onMsg
    tcpServer.onCloseFunc=_P.onClose

    assert(tcpServer:listen(port,host),string.format("listen %s:%d failed!",host,port))
    api.info(string.format("cluster %s:%d start!",host,port))

    api.dispatch(api.MsgType.Lua,_P.onDispatch)
end)

api.shutdown(function()
    if tcpClients then
        for _,it in pairs(tcpClients)do
            it:destroy()
        end
    end

    if tcpServer then
        tcpServer:destroy()
    end
end)