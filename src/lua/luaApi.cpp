#include"lua.hpp"
#include "luaApi.hpp"
#include<string>
#include <map>

bool IsRun = true;
std::map<std::string, std::string> Envs;
std::string CPath;
std::string LuaPath;

extern void luaRegisterJsonAPI(lua_State* state);
extern void luaRegisterMariadbAPI(lua_State* state);
extern void luaRegisterLogAPI(lua_State* state);
extern void luaRegisterServiceAPI(lua_State* state);
extern void luaRegisterHttpServerAPI(lua_State* state);
extern void luaRegisterHttpClientAPI(lua_State* state);
extern void luaRegisterWebsocketServerAPI(lua_State* state);
extern void luaRegisterTcpServerAPI(lua_State* state);
extern void luaRegisterTcpClientAPI(lua_State* state);

int exit(lua_State* L)
{
	IsRun = false;
	return 0;
}

void luaExit()
{
	IsRun = false;
}

void luaRegisterCoreAPI(lua_State* state)
{
	lua_settop(state, 0);

	auto name = "core";
	auto hasTable = true;
	lua_getglobal(state, name);
	if (lua_isnil(state, -1))
	{
		lua_newtable(state);
		hasTable = false;
	}

	lua_pushcfunction(state, exit);
	lua_setfield(state, -2, "exit");

	if (!hasTable)
	{
		lua_setglobal(state, name);
	}
	else
	{
		lua_pop(state, 1);
	}
}

void luaRegisterAPI(lua_State* state)
{
#ifdef _DEBUG
	lua_pushboolean(state, true);
#else
	lua_pushboolean(state, false);
#endif

	lua_setglobal(state, "DEBUG_MODE");

	luaRegisterCoreAPI(state);
	luaRegisterServiceAPI(state);
	luaRegisterLogAPI(state);
	luaRegisterJsonAPI(state);
	luaRegisterMariadbAPI(state);
	luaRegisterHttpServerAPI(state);
	luaRegisterHttpClientAPI(state);
	luaRegisterWebsocketServerAPI(state);
	luaRegisterTcpServerAPI(state);
	luaRegisterTcpClientAPI(state);
}

void luaSetCPath(std::string path)
{
	CPath = path;
}

void luaSetLuaPath(std::string path)
{
	LuaPath = path;
}

bool luaIsRun()
{
	return IsRun;
}

lua_State* luaNewState()
{
	auto L = luaL_newstate();
	luaL_openlibs(L);

	lua_getglobal(L, "package");

	lua_pushstring(L, LuaPath.c_str());
	lua_setfield(L, -2, "path");

	lua_pushstring(L, CPath.c_str());
	lua_setfield(L, -2, "cpath");

	lua_pop(L, 1);

	luaRegisterAPI(L);

	return L;
}

void luaPrintStack(lua_State* state)
{
	printf("==============Lua Stack ==============\n");
	auto size = lua_gettop(state);
	for (auto i = size; i > 0; i--)
	{
		auto typeId = lua_type(state, i);
		auto typeName = lua_typename(state, typeId);
		auto valueStr = std::string();
		switch (typeId)
		{
		case LUA_TNONE:
			valueStr = "none";
			break;
		case LUA_TNIL:
			valueStr = "nil";
			break;
		case LUA_TBOOLEAN:
			valueStr = std::to_string(lua_toboolean(state, i));
			break;
		case LUA_TLIGHTUSERDATA:
			valueStr = std::to_string((int64_t)lua_touserdata(state, i));
			break;
		case LUA_TNUMBER:
			valueStr = std::to_string(lua_tonumber(state, i));
			break;
		case LUA_TSTRING:
			valueStr = std::string(lua_tostring(state, i));
			break;
		case LUA_TTABLE:
			valueStr = "table";
			break;
		case LUA_TFUNCTION:
			valueStr = "function";
			break;
		case LUA_TUSERDATA:
			valueStr = std::to_string((int64_t)lua_touserdata(state, i));
			break;
		case LUA_TTHREAD:
			valueStr = "coroutine";
			break;
		}

		printf("%d %d (%s)%s\n", i - size - 1, i, typeName, valueStr.c_str());
	}
	printf("==============Lua Stack End==============\n\n");
}