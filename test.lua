-- local api=require "api"

-- api.newService(
--     {
--         name="testService",
--         src="testService.lua",
--         unique=false,
--     }
-- )

-- api.newService(
--     {
--         name="testService2",
--         src="testService2.lua",
--         unique=true,
--     }
-- )

local id=httpServer.new(function(request)
    local response=
    {
        status=200,
        msg="Hello World!"
    }

    return response
end)
httpServer.listen(id,8080)