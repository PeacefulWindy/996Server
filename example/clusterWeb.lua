local api=require "api"

api.newService("clusterWeb","clusterWeb.lua",true,{port=6000,config="example/clusterNode.json"})