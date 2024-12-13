local _M={}
local _P={}
local hotfix=require "hotfix"
local anotherMod=require "repeatRequre"

local hotfixFlag=0

local value=3+hotfixFlag

_M.value=1+hotfixFlag

function _M.printValue()
    return 2+hotfixFlag
end

function _M.printLocalValue()
    return value
end

function _M.printPrivateValue()
    return _P.getValue()
end

function _M.printGlobalFuncValue()
    return getValue()
end

function _P.getValue()
    return 4+hotfixFlag
end

function getValue()
    return 5+hotfixFlag
end

return _M