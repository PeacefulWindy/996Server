local api=require "api"

api.newService(
    {
        name="testService",
        src="testService.lua",
        unique=false,
    }
)

api.newService(
    {
        name="testService2",
        src="testService2.lua",
        unique=true,
    }
)