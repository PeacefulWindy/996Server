local api=require "api"

api.async(function()
    print(api.call(api.MsgType.Lua,1,"AAA","BBB","CCC"))
end)