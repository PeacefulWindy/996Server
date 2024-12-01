-- print(...)--所有的参数将会以...传递
Config=
{
    luaPath="lualib/?.lua;./?.lua",--lua检索路径，跟package.path挂钩
    cPath="libs/?.so;libs/?.dll",--dll/so检索路径，跟package.cpath挂钩
    start="test.lua",--lua启动脚本
    thread=2,--CPU线程数(可选，默认为系统线程数)
    logLevel="debug",--日志等级(debug,info,warn,error)(可选,debug模式最低为debug,release模式最低为info)
    logFile=string.format("log/%s.log",os.date("%Y-%m-%d %H-%M-%S")),--日志文件路径(可选，默认不生成)
    servicePath=--lua服务路径，跟require没有关系
    {
        "services"
    },
    args=--启动参数。将会以...传递到对应start参数的lua脚本
    {
        "argA",
        "argB",
    },
}