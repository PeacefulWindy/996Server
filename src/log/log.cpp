#include<lua/luaApi.hpp>
#include<spdlog/spdlog.h>

int debug(lua_State* state)
{
	auto data = lua_tostring(state, 1);
	spdlog::debug("{}", data);

	return 0;
}

int info(lua_State* state)
{
	auto data = lua_tostring(state, 1);
	spdlog::info("{}", data);
	return 0;
}

int warning(lua_State* state)
{
	auto data = lua_tostring(state, 1);
	spdlog::warn("{}", data);
	return 0;
}

int error(lua_State* state)
{
	auto data = lua_tostring(state, 1);
	spdlog::error("{}", data);
	return 0;
}

void luaRegisterLogAPI(lua_State* state)
{
	lua_settop(state, 0);

	lua_newtable(state);

	lua_pushcfunction(state, debug);
	lua_setfield(state, -2, "debug");

	lua_pushcfunction(state, info);
	lua_setfield(state, -2, "info");

	lua_pushcfunction(state, warning);
	lua_setfield(state, -2, "warning");

	lua_pushcfunction(state, error);
	lua_setfield(state, -2, "error");

	lua_setglobal(state, "log");
}