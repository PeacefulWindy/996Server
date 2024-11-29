local class=require "class"
local api=require "api"
local _M=class()

---@param host string
---@param port string
function _M:ctor(host,port)
    self.ptr=redis.new(host,port)
    assert(self.ptr)
end

function _M:destroy()
    redis.destroy(self.ptr)
    self.ptr=nil
end

---@param key string
---@return boolean
function _M:exists(key)
    local cmd=string.format("exists %s",key)
    local isOk,ret=redis.cmd(self.ptr,cmd)
    if not ret or ret[1] ~= 1 then
        return false
    end

    return true
end

---@param key string
---@return boolean
function _M:del(key)
    local cmd=string.format("del %s",key)
    local ret=redis.cmd(self.ptr,cmd)
    if not ret or ret[1] ~= 1 then
        return false
    end

    return true
end

---@param key string
---@param time integer @ second
---@return boolean
function _M:expire(key,time)
    local cmd=string.format("expire %s %d",key,time)
    local ret=redis.cmd(self.ptr,cmd)
    if not ret or ret[1] ~= 1 then
        return false
    end

    return true
end

---@param key string
---@return boolean
function _M:ttl(key)
    local cmd=string.format("ttl %s",key)
    local ret=redis.cmd(self.ptr,cmd)
    if not ret then
        return -2
    end

    return ret[1]
end
---@param key string
---@return string
function _M:type(key)
    local cmd=string.format("type %s",key)
    local ret=redis.cmd(self.ptr,cmd)
    if not ret then
        return "none"
    end

    return ret[1]
end

---@param key string
---@return string|integer|boolean|number
function _M:get(key)
    local cmd=string.format("get %s",key)
    local ret=redis.cmd(self.ptr,cmd)
    if not ret then
        return
    end

    return ret[1]
end

---@param key string
---@param value string|integer|boolean|number
---@return boolean
function _M:set(key,value)
    local cmd=string.format("set %s %s",key,value)
    local ret=redis.cmd(self.ptr,cmd)
    if not ret or not ret[1] ~= "OK" then
        return false
    end

    return true
end

---@param keys table
---@return table
function _M:mget(keys)
    local cmd=string.format("mget %s",table.concat(keys," "))
    local ret=redis.cmd(self.ptr,cmd)
    if not ret then
        return {}
    end

    return ret
end

---@param dataMap table @key-value
---@return boolean
function _M:mset(dataMap)
    local tb={}
    for k,v in pairs(dataMap)do
        table.insert(tb,string.format("%s %s",k,v))
    end
    local cmd=string.format("mset %s",table.concat(tb," "))
    local ret=redis.cmd(self.ptr,cmd)
    if not ret or not ret[1] ~= "OK" then
        return false
    end

    return true
end

---@param key string
---@param field string
---@return boolean
function _M:hdel(key,field)
    local cmd=string.format("hdel %s %s",key,field)
    local ret=redis.cmd(self.ptr,cmd)
    if not ret or ret[1] ~= 1 then
        return false
    end

    return true
end

---@param key string
---@param field string
---@return boolean
function _M:hexists(key,field)
    local cmd=string.format("hexists %s %s",key,field)
    local ret=redis.cmd(self.ptr,cmd)
    if not ret or ret[1] ~= 1 then
        return false
    end

    return true
end

---@param key string
---@param field string
---@return string|integer|number|boolean
function _M:hget(key,field)
    local cmd=string.format("hget %s %s",key,field)
    local ret=redis.cmd(self.ptr,cmd)
    if not ret then
        return
    end

    return ret[1]
end

---@param key string
---@return table
function _M:hgetall(key)
    local cmd=string.format("hgetall %s",key)
    return redis.cmd(self.ptr,cmd)
end
---@param key string
---@param field string
---@param value string|integer|number|boolean
---@return boolean
function _M:hset(key,field,value)
    local cmd=string.format("hset %s %s %s",key,field,value)
    local ret=redis.cmd(self.ptr,cmd)
    if not ret or ret[1] ~= "OK" then
        return false
    end

    return true
end

---@param key string
---@param dataMap table @key-value
---@return boolean
function _M:hmset(key,dataMap)
    local tb={}
    for k,v in pairs(dataMap)do
        table.insert(tb,string.format("%s %s",k,v))
    end

    local cmd=string.format("hmset %s %s",key,table.concat(tb," "))
    local ret=redis.cmd(self.ptr,cmd)
    if not ret or ret[1] ~= "OK" then
        return false
    end

    return true
end

---@param key string
---@return table
function _M:hkeys(key)
    local cmd=string.format("hkeys %s",key)
    return redis.cmd(self.ptr,cmd)
end

return _M