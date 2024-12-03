local api=require "api"
local sysArgs={...}

api.setEnv("node",sysArgs[1])
local clusterId=api.newService("cluster","clusterServ.lua",true,{url="http://127.0.0.1:6000/cluster?id=%s"})
if clusterId <= 0 then
    api.error("clusterServ new failed!")
    api.exit()
end

api.newService("clusterSender","clusterSender.lua",true)