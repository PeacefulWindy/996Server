local api=require "api"

api.newService("clusterWeb","clusterWebServ.lua",true,{port=6000,config="example/clusterNode.json"})