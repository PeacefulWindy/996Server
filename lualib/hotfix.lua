local api=require "api"
local _M={}
local _P=
{
    mods={},
}

function _P.getLuaPaths(filePath)
    local tb=api.split(package.path,";")
    for k,v in pairs(tb)do
        local filePath=string.gsub(v,"%?",filePath)
        tb[k]=filePath
    end

    return tb
end

function _M.require(path)
    local filePath=string.gsub(path,"%.","/")
    local ext=fs.getFileExtension(filePath)
    filePath=filePath:gsub(ext,"")

    if not package.loaded[path] then
        local luaPaths=_P.getLuaPaths(filePath)
        for _,it in pairs(luaPaths)do
            local fileFunc=loadfile(it)
            if fileFunc then
                local proxyMod={}
                setmetatable(proxyMod,
                {
                    __index=function()
                        assert(false,string.format("%s was loading",path))
                    end,
                    __newIndex=function()
                        assert(false,string.format("%s was loading",path))
                    end
                })

                package.loaded[path]=proxyMod

                local isOk,ret=pcall(fileFunc)
                if not isOk then
                    api.error(ret)
                    setmetatable(proxyMod,
                    {
                        __index=function()
                            assert(false,string.format("%s load failed!",path))
                        end,
                        __newIndex=function()
                            assert(false,string.format("%s load failed!",path))
                        end
                    })
                    return
                end

                if type(ret) ~= "table" then
                    api.error("invalid lua file!you must return table!")
                    setmetatable(proxyMod,
                    {
                        __index=function()
                            assert(false,string.format("%s load failed!",path))
                        end,
                        __newIndex=function()
                            assert(false,string.format("%s load failed!",path))
                        end
                    })
                    return
                end

                local mod=ret
                setmetatable(proxyMod,
                {
                    __index=mod,
                    __newIndex=mod
                })

                _P.mods[path]=
                {
                    mod=mod,
                    filePath=it,
                }

                return proxyMod
            end
        end

        api.error(string.format("not found %s\n\t%s",path,table.concat(luaPaths,"\n\t")))
    end

    return package.loaded[path]
end

function _M.reload()
    for path,modInfo in pairs(_P.mods)do
        local proxy=package.loaded[path]
        if not proxy then
            return false
        end

        local fileFunc=loadfile(modInfo.filePath)
        if fileFunc then
            local isOk,ret=pcall(fileFunc)
            if not isOk then
                api.error(ret)
                return false
            end

            local mod=modInfo.mod
            for k,v in pairs(mod)do
                if ret[k] then
                    mod[k]=ret[k]
                end
            end

            if mod.onReload then
                local isOk=pcall(mod.onReload)
                if not isOk then
                    return false
                end
            end
        end
    end

    return true
end

_M.nativeRequire=require
require=_M.require

return _M 