local api=
{
    MsgType=
    {
        Close=1,
        Lua=2,
        LuaResponse=3,
        HttpClient=4,
        WebsocketServer=5,
    },
    websocketServers={},
    httpCoro={},
    nextCoro={},
    callCoro={},
    protocolRegister={},
    autoSessionId=1,
    callTimeout=20,
    closeFunc=nil
}

local _P={}

function _P.formatLogArgs(...)
    local tb={...}
    local args={}
    for _,it in pairs(tb)do
        table.insert(args,tostring(it))
    end
    return args
end

function api.debug(...)
    local tb=_P.formatLogArgs(...)
    log.debug(string.format("[%s]:%s",SERVICE_NAME or "main",table.concat(tb,"\t")))
end
api.print=print
print=api.debug

function api.info(...)
    local tb=_P.formatLogArgs(...)
    log.info(string.format("[%s]:%s",SERVICE_NAME or "main",table.concat(tb,"\t")))
end

function api.warn(...)
    local tb=_P.formatLogArgs(...)
    log.warning(string.format("[%s]:%s",SERVICE_NAME or "main",table.concat(tb,"\t")))
end

function api.error(...)
    local tb=_P.formatLogArgs(...)
    log.error(string.format("[%s]:%s",SERVICE_NAME or "main",table.concat(tb,"\t")))
end

function api.copyTable(tb)
    local ret={}

    for k,v in pairs(tb)do
        if type(v) == "table" then
            ret[k]=_M.copyTable(v)
        else
            ret[k]=v
        end
    end

    return ret
end

function api.dumpTable(tb,name,layer)
    if not tb then
        return
    end

    if not name then
        name=""
    end

    if not layer then
        layer=0
    end

    if layer == 0 then
        print("==========",name," Start==========")
    end

    for k,v in pairs(tb)do
        local spaceTb={}
        for i=1,layer do
            table.insert(spaceTb,"\t")
        end

        if type(v) == "table" then
            print(name..table.concat(spaceTb," ").."["..type(k).."]"..k.." => ")
            api.dumpTable(v,name,layer+1)
        else
            print(name..table.concat(spaceTb," ").."["..type(k).."]"..k.." = ["..type(v).."]"..tostring(v))
        end
    end

    if layer == 0 then
        print("==========",name," End==========")
    end
end

function api.exit()
    core.exit()
end

---@param args table
---@return number
function api.newService(args)
    if not args.name then
        log.error("invalid newService name!")
        return
    end

    if not args.src then
        log.error("invalid newService src!")
        return
    end

    local name=args.name
    local src=args.src
    local isUnique=args.unique or false

    return core.newService(name,src,isUnique)
end

---@param name string
---@return number
function api.queryService(name)
    return core.queryService(name)
end

---@param id integer
function api.destroyService(id)
    core.send(id,api.MsgType.Close,0,"",0)
end

function api.async(func)
    local co=coroutine.create(func)
    table.insert(api.nextCoro,co)
end

function api.dispatch(type,func)
    api.protocolRegister[type]=func
end

function api.send(msgType,serviceId,...)
    local args={...}
    local data=json.encode(args)
    core.send(serviceId,msgType,0,data,#data)
end

function api.response(serviceId,sessionId,...)
    local args={...}
    local data=json.encode(args)
    core.send(serviceId,api.MsgType.LuaResponse,sessionId,data,#data)
end

function api.call(msgType,serviceId,...)
    local sessionId=api.autoSessionId
    api.autoSessionId=api.autoSessionId+1
    local co=coroutine.running()
    api.callCoro[sessionId]=
    {
        co=co,
        time=os.time()+api.callTimeout,
    }

    local args={...}
    local data=json.encode(args)
    core.send(serviceId,msgType,sessionId,data,#data)
    local status,msg=coroutine.yield(sessionId)
    if not status then
        api.error("call failed!\n",debug.traceback())
        return false
    end

    return true,table.unpack(msg)
end

function api.quit()
    core.exit()
end

function onPoll()
    for i=#api.nextCoro,1,-1 do
        local co=api.nextCoro[i]
        table.remove(api.nextCoro,i)
        local status,err=coroutine.resume(co)
        if not status then
            api.error(err)
        end
    end

    local now=os.time()
    --处理消息
    for k,v in pairs(api.callCoro) do
        if now > v.time then
            local status,err=coroutine.resume(v.co,false)
            if not status then
                api.error(err)
            end
            api.callCoro[k]=nil
        end
    end

    local msgs=core.getAllMsg()
    local isExit=false
    for _,it in pairs(msgs)do
        if it.msgType == api.MsgType.Close then
            isExit=true
        elseif it.msgType == api.MsgType.HttpClient then
            local co=api.httpCoro[it.session]
            if co then
                local status,err=coroutine.resume(co,{data=it.data,status=it.status,error=it.error})
                if not status then
                    api.error(err)
                end
            end
        elseif it.msgType == api.MsgType.WebsocketServer then
            local inst=api.websocketServers[it.session]
            if not inst then
                return
            end

            inst:onMsg(it.fd,it.status,it.data)
        else
            local func=api.protocolRegister[it.msgType]
            if func then
                func(it.source,it.session,table.unpack(json.decode(it.data)))
            end
        end
    end

    if isExit then
        api.async(function()
            if api.closeFunc then
                api.closeFunc()
            end

            core.destoryService(SERVICE_ID)
        end)
    end
end

function api.shutdown(func)
    api.closeFunc=func
end

api.dispatch(api.MsgType.LuaResponse,function(_,session,...)
    local coInfo=_P.callCoro[session]
    if not coInfo then
        return
    end

    _P.callCoro[session]=nil
    local status,err=coroutine.resume(coInfo.co,true,{...})
    if not status then
        api.error(err)
    end
end)

return api