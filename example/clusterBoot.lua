local api=require "api"
local sysArgs={...}

local serviceName=sysArgs[1]
local servicePath=sysArgs[2]

table.remove(sysArgs,2)
table.remove(sysArgs,1)

api.newService(serviceName,servicePath,true,sysArgs)