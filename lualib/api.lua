local api={}
local _P=
{
    nextCoro={},
    autoSessionId=1,
}

function _P.formatLogArgs(...)
    local tb={...}
    local args={}
    for _,it in pairs(tb)do
        if it == nil then
            table.insert(args,"nil")
        elseif it == false then
            table.insert(args,"false")
        elseif it == true then
            table.insert(args,"true")
        else
            table.insert(args,it)
        end
    end
    return args
end

function api.debug(...)
    local tb=_P.formatLogArgs(...)
    log.debug(table.concat(tb,"\t"))
end
api.print=print
print=api.debug

function api.info(...)
    local tb=_P.formatLogArgs(...)
    log.info(table.concat(tb,"\t"))
end

function api.warn(...)
    local tb=_P.formatLogArgs(...)
    log.warning(table.concat(tb,"\t"))
end

function api.error(...)
    local tb=_P.formatLogArgs(...)
    log.error(table.concat(tb,"\t"))
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

function api.send()

end

function api.call()

end

function api.quit()

end

function api.onQuit()
    
end

function onPoll()
    for i=#_P.nextCoro,1,-1 do
        local co=_P.nextCoro[i]
        coroutine.resume(co)
        if coroutine.status(co) == "dead" then
            table.remove(_P.nextCoro,i)
        end
    end
end

function onDestroy()
    
end

return api