Config=
{
    luaPath="lualib/?.lua;./?.lua",
    cPath="libs/?.so;libs/?.dll",
    start="example/clusterReceiver.lua",
    thread=2,
    servicePath=
    {
        "example/services",
        "services"
    },
    args=
    {
        1
    },
}