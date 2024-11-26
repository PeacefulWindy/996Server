require "class"
local api=require "api"
local _M={}

function _M.get(url,headers)
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

function _M.post(url,form,headers)
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

return _M