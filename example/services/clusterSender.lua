local api=require "api"
local cluster=require "cluster"
local SysArgs=api.args()

api.timer(500,function()
    api.async(function()
        local text="Hello World!"
        api.info("Send:",text)
        local isOk,response=cluster.call(1,"receiver",text)
        if not isOk then
            return
        end

        api.info("Receiver:",response)
    end)
end)