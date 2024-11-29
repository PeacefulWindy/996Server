local _M={}

---@param dirPath string
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

        local filePath=string.format("%s/%s",dirPath,fileName)
        local file=io.open(filePath,"r")
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
    end

    config.init(configs)
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