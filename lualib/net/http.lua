local class=require "class"
local api=require "api"
local _M={}
local _P={}

---@param url string
---@param headers? string
---@return table
function _M.get(url,headers)
    if not url then
        return
    end

    local args=
    {
        url=url,
        headers=headers,
    }

    local sessionId=api.autoSessionId
    api.autoSessionId=api.autoSessionId+1

    local ret=httpClient.get(sessionId,args)
    if not ret then
        return
    end

    local co=coroutine.running()
    api.httpCoro[sessionId]=co
    local response=coroutine.yield()
    return response
end

---@param url string
---@param form? table
---@param headers? string
---@return table
function _M.post(url,form,headers)
    if not url then
        return
    end

    local args=
    {
        url=url,
        headers=headers,
    }

    local sessionId=api.autoSessionId
    api.autoSessionId=api.autoSessionId+1

    local ret=httpClient.post(sessionId,form or {},args)
    if not ret then
        return
    end

    local co=coroutine.running()
    api.httpCoro[sessionId]=co
    local response=coroutine.yield()
    return response
end

local HttpServer=class("HttpServer")
local tcp=require "net.tcp"

function _P.parseHttpRequest(msg)
    local lines=api.split(msg,"%\r%\n")
    assert(#lines>1,"invalid request")

    local request=
    {
        headers={},
        body={},
    }

    for i=1,#lines-2 do
        local line=lines[i]
        if i == 1 then
            local data=api.split(line," ")
            request.method=data[1]
            request.uri=data[2]
            request.version=data[3]
            local argIndex=request.uri:find("%?")
            if argIndex then
                local args=api.split(request.uri:sub(argIndex+1,#request.uri),"&")
                request.uri=request.uri:sub(0,argIndex-1)

                if args[1] ~= request.uri then
                    for _,it in pairs(args)do
                        local index=it:find("=")
                        if index then
                            local key=it:sub(0,index-1)
                            local value=it:sub(index,#line)
                            request.body[key]=value
                        end
                    end 
                end
            end
        else
            local index=line:find("%: ")
            if index then
                local key=line:sub(0,index-1)
                local value=line:sub(index,#line)
                request.headers[key]=value
            end
        end
    end

    return request
end

_P.StatusCodeStr=
{
    [100] = "Continue",
    [101] = "Switching Protocols",
    [200] = "OK",
    [201] = "Created",
    [202] = "Accepted",
    [203] = "Non-Authoritative Information",
    [204] = "No Content",
    [205] = "Reset Content",
    [206] = "Partial Content",
    [300] = "Multiple Choices",
    [301] = "Moved Permanently",
    [302] = "Found",
    [303] = "See Other",
    [304] = "Not Modified",
    [305] = "Use Proxy",
    [307] = "Temporary Redirect",
    [400] = "Bad Request",
    [401] = "Unauthorized",
    [402] = "Payment Required",
    [403] = "Forbidden",
    [404] = "Not Found",
    [405] = "Method Not Allowed",
    [406] = "Not Acceptable",
    [407] = "Proxy Authentication Required",
    [408] = "Request Time-out",
    [409] = "Conflict",
    [410] = "Gone",
    [411] = "Length Required",
    [412] = "Precondition Failed",
    [413] = "Request Entity Too Large",
    [414] = "Request-URI Too Large",
    [415] = "Unsupported Media Type",
    [416] = "Requested range not satisfiable",
    [417] = "Expectation Failed",
    [500] = "Internal Server Error",
    [501] = "Not Implemented",
    [502] = "Bad Gateway",
    [503] = "Service Unavailable",
    [504] = "Gateway Time-out",
    [505] = "HTTP Version not supported",
}

_P.responseTemplate="HTTP/1.1 %s %s\r\n%s\r\n\r\n%s"

function _P.formatHttpResponse(msg)
    if not msg.headers then
        msg.headers={}
    end
    
    local statusCode=msg.status or 200
    local body=msg.body or ""
    local headers={}
    if msg.headers then
        for k,v in pairs(msg.headers)do
            table.insert(headers,string.format("%s:%s",k,v))
        end
    end

    if not msg.headers["Content-Type"] then
        table.insert(headers,string.format("%s:%s","Content-Type","text/plain"))
    end

    local bodyLen=#body
    table.insert(headers,string.format("%s:%d","Content-length",bodyLen))

    if not msg.headers["Connection"] then
        table.insert(headers,string.format("%s:%s","Connection","close"))
    end

    return string.format(_P.responseTemplate,statusCode,_P.StatusCodeStr[statusCode],table.concat(headers,"\r\n"),body)
end

function HttpServer:ctor()
    self.router={}
    self.ptr=tcp.newServer()
    self.ptr.onMsgFunc=function(_,fd,msg)
        local ret
        local isOk,request=pcall(_P.parseHttpRequest,msg)
        if not isOk then
            api.error(request)
            ret={status=400}
        else
            local func=self.router[request.uri]
            if func then
                local isOk,response=pcall(func,request)
                if not isOk then
                    api.error(response)
                    ret={status=500}
                else
                    ret=response
                end
            else
                ret={status=404}
            end
        end

        if not ret or type(ret) ~= "table" then
            ret={status=200}
        end

        local isOk,response=pcall(_P.formatHttpResponse,ret)
        if not isOk then
            api.error(response)

            if ret.status ~= 500 then
                response=_P.formatHttpResponse({status=500})
            end
        end

        self.ptr:send(fd,response)
    end
end

function HttpServer:destroy()
    self.ptr:destroy()
end

---@param uri string
---@param func fun(request:string)
function HttpServer:on(uri,func)
    self.router[uri]=func
end

---@param port integer
---@return boolean
function HttpServer:listen(port)
    return self.ptr:listen(port)
end

function _M.newHttpServer()
    return HttpServer.new()
end

return _M