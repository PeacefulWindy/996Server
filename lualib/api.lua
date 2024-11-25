local api={}

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
    local isUnique=args.isUnique or false

    return core.newService(name,src,isUnique)
end

---@param name string
---@return number
function api.queryServices(name)
    return core.queryServices(name)
end

---@param id integer
function api.destroyService(id)
    core.destroyServices(id)
end

function api.async()

end

function api.quit()

end

function api.onQuit()
    
end

return api