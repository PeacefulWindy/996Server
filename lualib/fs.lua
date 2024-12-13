log.error("DO NOT REQUIRE THIS FILE")
local _M={}

---@param dirPath string
---@return table
function _M.listdir(dirPath)end

---@param path string
---@return boolean
function _M.exists(path)end

---@param path string
---@return boolean,string
function _M.mkdir(path)end

---@param path string
---@return boolean,string
function _M.rmdir(path)end

---@param path string
---@return string
function _M.getFileName(path)end

---@param path string
---@return string
function _M.getFileExtension(path)end

return _M