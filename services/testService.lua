local api=require "api"

api.async(function()
    api.dispatch(api.MsgType.Lua,function(source,session,...)
        print(source,session,...)
        api.response(source,session,...)
    end)
end)