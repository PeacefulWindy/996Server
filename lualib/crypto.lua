log.error("DO NOT REQUIRE THIS FILE")

local _M={}

---@param str string
---@return string
function _M.base64Encode(str)end

---@param str string
---@return string
function _M.base64Decode(str)end

---@param keyLen integer
---@return string,string @publicKey,privateKey
function _M.rsaGen(keyLen)end

---@param publicKey string
---@param data string
---@return string
function _M.rsaEncode(publicKey,data)end

---@param privateKey string
---@param data string
---@return string
function _M.rsaDecode(privateKey,data)end

---@param data string
---@return string
function _M.sha3_256(data)end

---@param data string
---@return string
function _M.sha3_512(data)end

return _M