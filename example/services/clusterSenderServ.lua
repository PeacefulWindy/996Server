local api=require "api"
local cluster=require "cluster"
local SysArgs=api.args()

api.async(function()
    local datas=
    {
        "Hello World!",
        true,
        10,
        20.1
    }

    for _,text in pairs(datas)do
        api.info("Send:",text)
        local isOk,response=cluster.call(1,"receiver",text)
        if not isOk then
            api.error(string.format("send %s failed!",text))
            return
        end

        api.info("Receiver:",response)
    end

    api.info("cluster test finish!")
    api.exit()
end)