local api=require "api"
local http=require "http"
local websocket=require "websocket"
local tcp=require "tcp"
local redis=require "redis"

api.async(function()
    local redis=redis.new("127.0.0.1",6379)
    local ret=redis:hkeys("AAA")
    api.dumpTable(ret)
    -- local isOk,ret=redis:get("A")
    -- api.dumpTable(ret)
    -- print(isOk,ret)
    -- local client=tcp.newClient()
    -- client.onConnectFunc=function()
    --     client:send("Hello World!")
    -- end
    -- client.onMsgFunc=function(_,data)
    --     print("AAA",data)
    --     client:close()
    -- end
    -- if not client:connect("127.0.0.1",8080) then
    --     return
    -- end
    -- local server=tcp.newServer()
    -- if not server:listen(6000) then
    --     return
    -- end
    -- server.onConnectFunc=function(_,fd)
    --     server:send(fd,"Hello!")
    -- end
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