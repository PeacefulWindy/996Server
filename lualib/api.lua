local api={}
local _P=
{
    nextCoro={},
    callCoro={},
    protocolRegister={},
    autoSessionId=1,
    callTimeout=1,
}

api.MsgType=
{
    Lua=1,
    LuaResponse=2,
}

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
    log.debug(string.format("[%s]:%s",SERVICE_NAME,table.concat(tb,"\t")))
end
api.print=print
print=api.debug

function api.info(...)
    local tb=_P.formatLogArgs(...)
    log.info(string.format("[%s]:%s",SERVICE_NAME,table.concat(tb,"\t")))
end

function api.warn(...)
    local tb=_P.formatLogArgs(...)
    log.warning(string.format("[%s]:%s",SERVICE_NAME,table.concat(tb,"\t")))
end

function api.error(...)
    local tb=_P.formatLogArgs(...)
    log.error(string.format("[%s]:%s",SERVICE_NAME,table.concat(tb,"\t")))
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

function api.dump(tb)
    function _M.dumpTable(tb,name,layer)
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
                _M.dumpTable(v,name,layer+1)
            else
                print(name..table.concat(spaceTb," ").."["..type(k).."]"..k.." = ["..type(v).."]"..tostring(v))
            end
        end
    
        if layer == 0 then
            print("==========",name," End==========")
        end
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
    core.destroyServices(id)
end

function api.async(func)
    local co=coroutine.create(func)
    table.insert(_P.nextCoro,co)
end

function api.dispatch(type,func)
    _P.protocolRegister[type]=func
end

function api.send(msgType,serviceId,...)
    local args={...}
    core.send(serviceId,msgType,0,json.encode(args))
end

function api.response(serviceId,sessionId,...)
    local args={...}
    core.send(serviceId,api.MsgType.LuaResponse,sessionId,json.encode(args))
end

function api.call(msgType,serviceId,...)
    local sessionId=_P.autoSessionId
    _P.autoSessionId=_P.autoSessionId+1
    local co=coroutine.running()
    _P.callCoro[sessionId]=
    {
        co=co,
        time=os.time()+_P.callTimeout,
    }

    local args={...}
    core.send(serviceId,msgType,sessionId,json.encode(args))
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
    for i=#_P.nextCoro,1,-1 do
        local co=_P.nextCoro[i]
        table.remove(_P.nextCoro,i)
        local status,err=coroutine.resume(co)
        if not status then
            api.error(err)
        end
    end

    local now=os.time()
    --处理消息
    for k,v in pairs(_P.callCoro) do
        if now > v.time then
            local status,err=coroutine.resume(v.co,false)
            if not status then
                api.error(err)
            end
            _P.callCoro[k]=nil
        end
    end

    local msgs=core.getAllMsg()
    for _,it in pairs(msgs)do
        local func=_P.protocolRegister[it.msgType]
        if func then
            func(it.source,it.session,table.unpack(json.decode(it.args)))
        end
    end
end

function onDestroy()
    
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