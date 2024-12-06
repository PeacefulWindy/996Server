local api=require "api"

api.async(function()
    api.dispatch(api.MsgType.Lua,function(source,session,...)
        api.info("Receiver:",...)
        api.response(source,session,true,...)
    end)
end)