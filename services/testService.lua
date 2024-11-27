local api=require "api"
local http=require "http"
local websocket=require "websocket"

api.async(function()
    local server=websocket.newServer()
    if not server:listen(6000) then
        return
    end
    server.onConnectFunc=function(_,fd)
        server:send(fd,"Hello!")
    end
    -- api.destroyService(SERVICE_ID)
    -- local response=http.post("http://127.0.0.1:8080",{user="AAA",pwd="BBB"})
    -- api.dumpTable(response)
    -- api.dispatch(api.MsgType.Lua,function(source,session,...)
    --     print(source,session,...)
    --     api.response(source,session,...)
    -- end)
end)

api.shutdown(function()
end)