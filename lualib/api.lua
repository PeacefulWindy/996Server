local api=
{
    MsgType=
    {
        Close=1,
        Timer=2,
        Lua=3,
        LuaResponse=4,
        HttpClient=5,
        WebsocketServer=6,
        WebsocketClient=7,
        TcpServer=8,
        TcpClient=9,
    },
    websocketServers={},
    tcpServers={},
    tcpClients={},
    httpCoro={},
    nextCoro={},
    callCoro={},
    timers={},
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

---@param key string
---@return string
function api.env(key)
    return core.env(key) or ""
end

---@param key string
---@param value string
function api.setEnv(key,value)
    core.setEnv(key,tostring(value))
end

---@param tb table
---@return table
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

---@param tb table
---@param name? string
---@param layer? integer
function api.dumpTable(tb,name,layer,printFunc)
    if not name then
        name=""
    end

    if not layer then
        layer=0
    end

    if not printFunc then
        printFunc=print
    end

    if layer == 0 then
        printFunc("==========",name," Start==========")
    end

    if not tb or type(tb) ~= "table" then
        printFunc("invalid table!")
        if layer == 0 then
            printFunc("==========",name," End==========")
        end
        return
    end
    for k,v in pairs(tb)do
        local spaceTb={}
        for i=1,layer do
            table.insert(spaceTb,"\t")
        end

        if type(v) == "table" then
            printFunc(name..table.concat(spaceTb," ").."["..type(k).."]"..k.." => ")
            api.dumpTable(v,name,layer+1,printFunc)
        else
            printFunc(name..table.concat(spaceTb," ").."["..type(k).."]"..k.." = ["..type(v).."]"..tostring(v))
        end
    end

    if layer == 0 then
        printFunc("==========",name," End==========")
    end
end

---@param text string
---@return string
function api.fromHex(text,space)
    if not space then
        space=""
    end

    local tb={}
    for i = 1, #text, 2+#space do
        local byte = tonumber(text:sub(i, i + 1), 16)
        if byte then
            table.insert(tb,string.char(byte))
        end
    end

    return table.concat(tb,"")
end

---@param text string
---@return string
function api.toHex(text)
    local tb={}
    for i=1,#text do
        table.insert(tb,string.format("%02X",text:byte(i)))
    end

    return table.concat(tb," ")
end

---@param text string
---@param sep string
---@return table
function api.split(text, sep)
    if not sep then
        return {text}
    end

    local t = {}
    for it in string.gmatch(text, "([^" .. sep .. "]+)") do
        table.insert(t, it)
    end
    return t
end

---@return integer
function api.time()
    return core.time()
end

---@return integer
function api.mstime()
    return core.mstime()
end

---@return table
function api.args()
    return json.decode(SERVICE_SYSARGS or {})
end

function api.exit()
    core.exit()
end

---@param name string
---@param src string
---@param isUnique? boolean @default false
---@param args? table
---@return integer
function api.newService(name,src,isUnique,args)
    if not name then
        log.error("invalid newService name!")
        return 0
    end

    if not src then
        log.error("invalid newService src!")
        return 0
    end

    local isUnique=isUnique or false
    local sysArgs=args or {}

    return core.newService(name,src,isUnique,json.encode(sysArgs))
end

---@param name string
---@return integer
function api.queryService(name)
    return core.queryService(name)
end

---@param id integer
function api.destroyService(id)
    core.send(id,api.MsgType.Close,0,false,"")
end

---@param func fun()
function api.async(func)
    local co=coroutine.create(func)
    table.insert(api.nextCoro,co)
end

---@param type integer @api.MsgType
---@param func fun(source:integer,session:integer,...)
function api.dispatch(type,func)
    api.protocolRegister[type]=func
end

---@param serviceId integer
function api.send(serviceId,...)
    local args={...}
    local data=json.encode(args)
    core.send(serviceId,api.MsgType.Lua,0,false,data)
end

---@param serviceId integer
---@param sessionId integer
---@param isOk boolean
function api.response(serviceId,sessionId,isOk,...)
    local args={...}
    local data=json.encode(args)
    core.send(serviceId,api.MsgType.LuaResponse,sessionId,isOk,data)
end

---@param serviceId integer
---@return boolean,string|... @false will return err msg,true will return ...
function api.call(serviceId,...)
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
    core.send(serviceId,api.MsgType.Lua,sessionId,false,data)
    local status,isOk,ret=coroutine.yield(sessionId)
    if not status then
        return false,ret
    end

    if not isOk then
        return false,ret
    end
    
    return true,table.unpack(ret)
end

function api.quit()
    core.exit()
end

function onPoll(msgs)
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

    local isExit=false
    for _,it in pairs(msgs)do
        if it.msgType == api.MsgType.Close then
            isExit=true
        elseif it.msgType == api.MsgType.Timer then
            local func=api.timers[it.session]
            if func then
                func()
            end
        elseif it.msgType == api.MsgType.HttpClient then
            local co=api.httpCoro[it.session]
            if co then
                local status,err=coroutine.resume(co,{body=it.data,status=it.status,error=it.error})
                if not status then
                    api.error(err)
                end
            end
        elseif it.msgType == api.MsgType.WebsocketServer then
            local inst=api.websocketServers[it.session]
            if inst then
                inst:onMsg(it.fd,it.status,it.data)
            end
        elseif it.msgType == api.MsgType.TcpServer then
            local inst=api.tcpServers[it.session]
            if inst then
                inst:onMsg(it.fd,it.status,it.data)
            end
        elseif it.msgType == api.MsgType.TcpClient then
            local inst=api.tcpClients[it.session]
            if inst then
                inst:onMsg(it.status,it.data,it.error)
            end
        elseif it.msgType == api.MsgType.LuaResponse then
            local coInfo=api.callCoro[it.session]
            if not coInfo then
                return
            end
        
            api.callCoro[it.session]=nil
            local status,err=coroutine.resume(coInfo.co,true,it.isOk,json.decode(it.data))
            if not status then
                api.error(err)
            end
        else
            local func=api.protocolRegister[it.msgType]
            if func then
                func(it.source,it.session,table.unpack(json.decode(it.data)))
            else
                api.error(string.format("not register msgType:%d",it.msgType))
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

---@param func fun()
function api.shutdown(func)
    api.closeFunc=func
end

---@param time integer @millseconds
---@param func fun()
function api.timer(time,func)
    local session=api.autoSessionId
    api.autoSessionId=api.autoSessionId+1
    api.timers[session]=func
    core.timer(session,time)
end

return api