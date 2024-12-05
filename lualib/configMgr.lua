local _M={}
local api=require "api"

---@param dirPath string
---@return boolean
function _M.init(dirPath)
    local configPaths=fs.listdir(dirPath)
    
    local configs={}
    for _,fileName in pairs(configPaths) do
        local index=fileName:find("%.")
        local configName
        if index then
            configName=fileName:sub(1,index-1)
        else
            configName=fileName
        end
        
        if configName ~= "init" then
            local filePath=string.format("%s/%s",dirPath,fileName)
            local file=io.open(filePath,"r")
            if not file then
                api.error(string.format("%s load failed!",filePath))
                return false
            end
            
            local fileData=file:read("*a")
            local config=json.decode(fileData)
            for _,datas in pairs(config)do
                for k,v in pairs(datas)do
                    if type(v) == "table" then
                        datas[k]=
                        {
                            data=json.encode(v)
                        }
                    end
                end
            end
    
            configs[configName]=config
            print(filePath,"load!")
        end
    end

    config.init(configs)

    return true
end

---@param configName string
---@return table
function _M.getConfig(configName)
    local cfg,jsonMaps=config.getConfig(configName)
    if not cfg then
        return
    end

    for id,jsonKeys in pairs(jsonMaps)do
        for _,it in pairs(jsonKeys)do
            cfg[id][it]=json.decode(cfg[id][it].data)
        end
    end

    return cfg
end

---@param configName string
---@param id string|integer
---@return table
function _M.getLine(configName,id)
    local line,jsonKeys=config.getLine(configName,tostring(id))
    if not line then
        return
    end

    for _,it in pairs(jsonKeys)do
        line[it]=json.decode(line[it].data)
    end

    return line
end

---@param configName string
---@param id string|integer
---@param key string
---@return integer|number|string|table
function _M.getValue(configName,id,key)
    local value=config.getValue(configName,tostring(id),key)
    if not value then
        return
    end

    if type(value) == "table" then
        value=json.decode(value.data)
    end

    return value
end

return _M