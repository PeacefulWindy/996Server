#include<lua/luaApi.hpp>
#include<service/serviceMgr.hpp>

int newService(lua_State * L)
{
	auto name = luaL_checkstring(L, 1);
	auto src = luaL_checkstring(L, 2);
	auto isUnique = lua_toboolean(L, 3);

	auto serviceMgr = ServiceMgr::getInst();
	auto id=serviceMgr->newService(name,src, isUnique);

	lua_pushinteger(L,id);
	return 1;
}

int queryService(lua_State* L)
{
	auto name = luaL_checkstring(L, 1);
	auto serviceMgr = ServiceMgr::getInst();
	serviceMgr->queryService(name);

	return 0;
}

int destroyService(lua_State* L)
{
	auto id = luaL_checkinteger(L, 1);
	auto serviceMgr = ServiceMgr::getInst();
	serviceMgr->destoryService(id);

	return 0;
}

void luaRegisterServiceAPI(lua_State* state)
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

	lua_pushcfunction(state, newService);
	lua_setfield(state, -2, "newService");

	lua_pushcfunction(state, queryService);
	lua_setfield(state, -2, "queryService");

	lua_pushcfunction(state, destroyService);
	lua_setfield(state, -2, "destroyService");

	if (!hasTable)
	{
		lua_setglobal(state, name);
	}
	else
	{
		lua_pop(state, 1);
	}
}