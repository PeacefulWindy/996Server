require "class"
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

return _M