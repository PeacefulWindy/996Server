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

                for _,it in pairs(ret)do
                    if type(it) ~= "table" then
                        api.warn("only support hotfix table!")
                    end
                end

                local mod=ret[1]
                setmetatable(proxyMod,
                {
                    __index=mod,
                    __newIndex=mod
                })

                _P.mods[path]=
                {
                    mods=ret,
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

            for index,mod in pairs(modInfo.mods)do
                local newMod=ret[index]
                if newMod and type(newMod) == "table" then
                    for k,v in pairs(mod)do
                        if k ~= "onReload" and newMod[k] then
                            mod[k]=newMod[k]
                        end
                    end
                end
            end

            local mod=modInfo.mods[1]
            if mod and mod.onReload then
                local isOk=api.xpcall(mod.onReload)
                if not isOk then
                    return false
                end
            end
        end
    end

    return true
end

return _M 