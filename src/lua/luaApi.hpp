#pragma once
#include<lua.hpp>
#include<string>

lua_State* luaNewState(std::string externLuaPath = "");
void luaPrintStack(lua_State* state);
void luaRegisterAPI(lua_State* state);
void luaSetCPath(std::string path);
void luaSetLuaPath(std::string path);
bool luaIsRun();
void luaExit();