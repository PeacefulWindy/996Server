local api=require "api"
local sysArgs={...}

api.setEnv("node",sysArgs[1])
api.newService("cluster","clusterServ.lua",true,{url="http://127.0.0.1:6000/cluster?id=%s"})

api.newService("receiver","clusterReceiver.lua",true)