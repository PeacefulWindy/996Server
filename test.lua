local api=require "api"
local http=require "http"

api.newService(
    {
        name="testService",
        src="testService.lua",
        unique=false,
        args=
        {
            A="B"
        }
    }
)

-- api.newService(
--     {
--         name="testService2",
--         src="testService2.lua",
--         unique=true,
--     }
-- )

-- local id=httpServer.new(function(request)

-- end)
-- httpServer.listen(id,8080)

-- print(httpClient.get("http://192.168.1.1",{},function(response)
--     api.dumpTable(response)
-- end))

