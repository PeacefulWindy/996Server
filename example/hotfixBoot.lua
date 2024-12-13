local api=require "api"

api.newService("hotfix","hotfixTestServ.lua",true)
api.newService("console","consoleServ.lua",true,{port=8000})