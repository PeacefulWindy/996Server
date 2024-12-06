local api=require "api"
local sysArgs=api.args()

api.async(function()
    api.setEnv("node",sysArgs[1])
    local clusterId=api.newService("cluster","clusterServ.lua",true,{url="http://127.0.0.1:6000/cluster?id=%s"})
    if not clusterId then
        api.error("clusterServ new failed!")
        api.exit()
        return
    end

    api.call(clusterId,"listen")

    api.newService("receiver","clusterReceiverServ.lua",true)
    api.destroyService(SERVICE_ID)
end)

