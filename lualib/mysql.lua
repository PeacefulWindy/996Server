-- local class=require "class"
-- local _M=class()

-- function _M:ctor()
--     self.ptr=mysql.new()
--     self.stmts={}
--     self.autoId=1
-- end

-- function _M:destroy()
--     for _,it in pairs(self.stmts)do
--         mysql.destroyStmt(it)
--     end

--     mysql.destroy(self.ptr)
-- end

-- function _M:connect(host,port,user,pwd,db)
--     return mysql.connect(self.ptr,host,port,user,pwd,db)
-- end

-- function _M:newStmt()
--     local id=self.autoId
--     self.autoId=self.autoId+1
--     local stmt=mysql.newStmt(self.ptr)
--     self.stmts[id]=stmt
--     return id
-- end

-- function _M:destroyStmt(stmtId)
--     local stmt=self.stmts[stmtId]
--     if not stmt then
--         return
--     end


-- end

-- function _M:prepare(stmtId,sql)
--     local stmt=self.stmts[stmtId]
--     if not stmt then
--         return
--     end

--     return mysql.prepare(self.ptr,stmt,sql)
-- end

-- function _M:exec(sql,...)
--     local args={...}
--     return mysql.exec(self.ptr,sql,args)
-- end

-- return _M