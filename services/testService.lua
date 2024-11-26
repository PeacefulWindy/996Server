local api=require "api"
local http=require "http"

api.async(function()
    local response=http.post("http://127.0.0.1:8080",{user="AAA",pwd="BBB"})
    api.dumpTable(response)
    -- api.dispatch(api.MsgType.Lua,function(source,session,...)
    --     print(source,session,...)
    --     api.response(source,session,...)
    -- end)
end)