local class=require "class"
local api=require "api"
local _M=class()

function _M:ctor()
    self.ptr=mysql.new()
    self.stmts={}
    self.autoId=1
end

function _M:destroy()
    for _,it in pairs(self.stmts)do
        mysql.stmtClose(it)
    end

    mysql.destroy(self.ptr)
end

---@param host string
---@param port string
---@param user string
---@param pwd string
---@param db string
---@return boolean,string @false return with error
function _M:connect(host,port,user,pwd,db)
    return mysql.connect(self.ptr,host,port,user,pwd,db)
end

---@param sql string
---@return table
function _M:query(sql)
    return mysql.query(self.ptr,sql)
end

---@param sql string
---@return integer|nil
function _M:prepare(sql)
    local isOk,ret=mysql.prepare(self.ptr,sql)
    if not isOk then
        api.error(ret)
        return
    end

    local id=self.autoId
    self.autoId=self.autoId+1
    self.stmts[id]=ret
    return id
end

---@param stmtId integer
function _M:stmtClose(stmtId)
    local stmt=self.stmts[stmtId]
    if not stmt then
        return
    end

    mysql.stmtClose(stmt)
end

---@param stmtId integer
---@return table
function _M:exec(stmtId,...)
    local stmt=self.stmts[stmtId]
    if not stmt then
        return {badresult=true,errno=0,error="bad stmtId"}
    end

    return mysql.exec(stmt,...)
end

return _M