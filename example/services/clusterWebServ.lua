local http=require "net.http"
local api=require "api"
local SysArgs=api.args()
local _P={}

function _P.loadConfig(configPath)
    local file=io.open(configPath,"r")
    if not file then
        return
    end

    local data=json.decode(file:read("*a"))
    file:close()
    return data
end

local httpServer

local config=_P.loadConfig(SysArgs.config)
assert(config,"load config failed!")

httpServer=http.newHttpServer()
httpServer:on("/cluster",function(request)
    local response={}
    local nodeInfo=config[request.query["id"]]
    if not nodeInfo then
        response.status=400
        return response
    end

    response.body=json.encode(nodeInfo)
    return response
end)

assert(httpServer:listen(SysArgs.port),"listen failed!")
api.info(string.format("http server 127.0.0.1:%d start!",SysArgs.port))

api.shutdown(function()
    if httpServer then
        httpServer:destroy()
    end
end)