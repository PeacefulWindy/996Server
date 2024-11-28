local api=require "api"
local http=require "http"
local websocket=require "websocket"
local tcp=require "tcp"
local redis=require "redis"
local proto=require "proto"

api.async(function()
    local pb=proto.new()
    if not pb:loadFile("Center.pb") then
        print("No!")
        return
    end

    local data=
    {
        roleInfos=
        {
            {
                id=1,
                name="TestName",
                sex=2,
                job=4,
            }
        }
    }
    local ret,data=pb:encode("Center.GetRoleRet",data)
    api.print("encode ret:",ret,data,#data)
    ret,data=pb:decode("Center.GetRoleRet",data)
    print("decode ret:",ret)
    api.dumpTable(data)
    -- print(crypto.base64Encode("Hello World!"))
    -- local redis=redis.new("127.0.0.1",6379)
    -- local ret=redis:hkeys("AAA")
    -- local isOk,ret=redis:get("A")
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