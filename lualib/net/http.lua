local class=require "class"
local api=require "api"
local _M={}

---@param url string
---@param headers? string
---@return table
function _M.get(url,headers)
    if not url then
        return
    end

    local args=
    {
        url=url,
        headers=headers,
    }

    local sessionId=api.autoSessionId
    api.autoSessionId=api.autoSessionId+1

    local ret=httpClient.get(sessionId,args)
    if not ret then
        return
    end

    local co=coroutine.running()
    api.httpCoro[sessionId]=co
    local response=coroutine.yield()
    return response
end

---@param url string
---@param form? table
---@param headers? string
---@return table
function _M.post(url,form,headers)
    if not url then
        return
    end

    local args=
    {
        url=url,
        headers=headers,
    }

    local sessionId=api.autoSessionId
    api.autoSessionId=api.autoSessionId+1

    local ret=httpClient.post(sessionId,form or {},args)
    if not ret then
        return
    end

    local co=coroutine.running()
    api.httpCoro[sessionId]=co
    local response=coroutine.yield()
    return response
end

local HttpServer=class("HttpServer")

function HttpServer:ctor()
    self.router={}
    self.ptr=httpServer.new(function(request)
        local func=self.router[request.uri]
        if not func then
            return
        end

        return func(request)
    end)
end

function HttpServer:destroy()
    httpServer.destroy(self.ptr)
end

---@param uri string
---@param func fun(request:string)
function HttpServer:on(uri,func)
    self.router[uri]=func
end

---@param port integer
---@return boolean
function HttpServer:listen(port)
    return httpServer.listen(self.ptr,port)
end

function _M.newHttpServer()
    return HttpServer.new()
end

return _M