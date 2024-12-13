local hotfix=require "hotfix"
local hotfixMod=require "hotfixMod"
local api=require "api"

function printModValue()
    api.info("hotfixMod.value:",hotfixMod.value)
    api.info("hotfixMod.printValue:",hotfixMod.printValue())
    api.info("hotfixMod.printLocalValue:",hotfixMod.printLocalValue())
    api.info("hotfixMod.printPrivateValue:",hotfixMod.printPrivateValue())
    api.info("hotfixMod.printGlobalFuncValue:",hotfixMod.printGlobalFuncValue())
end

api.async(function()
    api.dispatch(api.MsgType.Lua,function(source,session,cmd)
        if cmd == "reload" then
            hotfix.reload()
            api.info("new value:")
            printModValue()
        end

        api.response(source,session,true)
    end)

    api.info('you can input "hotfix reload" to console!')
    api.info("old value:")
    printModValue()
end)