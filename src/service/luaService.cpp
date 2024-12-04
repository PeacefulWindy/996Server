#include<lua/luaApi.hpp>
#include<service/serviceMgr.hpp>
#include<service/service.hpp>
#include<service/msg/serviceMsgPool.hpp>
#include<cstring>

int newService(lua_State * L)
{
	auto name = luaL_checkstring(L, 1);
	auto src = luaL_checkstring(L, 2);
	auto isUnique = lua_toboolean(L, 3);
	auto args = luaL_checkstring(L, 4);

	auto serviceMgr = ServiceMgr::getInst();
	auto id = serviceMgr->newService(name, src, args, isUnique);

	lua_pushinteger(L,id);
	return 1;
}

int queryService(lua_State* L)
{
	auto name = luaL_checkstring(L, 1);
	auto serviceMgr = ServiceMgr::getInst();
	auto id = serviceMgr->queryService(name);

	lua_pushinteger(L, id);
	return 1;
}

int destoryService(lua_State* L)
{
	auto id = luaL_checkinteger(L, 1);
	auto serviceMgr = ServiceMgr::getInst();
	serviceMgr->destoryService(id);

	return 0;
}

int sendService(lua_State* L)
{
	auto target = luaL_checkinteger(L, 1);
	auto msgType = luaL_checkinteger(L, 2);
	auto session= luaL_checkinteger(L, 3);
	auto isOk = lua_toboolean(L, 4);
	auto data = luaL_checkstring(L, 5);
	auto dataLen = luaL_len(L, 5);

	lua_getglobal(L, "SERVICE_ID");
	auto serviceId = luaL_checkinteger(L, -1);

	if (msgType <= 0)
	{
		return 0;
	}

	auto serviceMgr = ServiceMgr::getInst();
	auto serviceMsgPool = ServiceMsgPool::getInst();
	auto msg = serviceMsgPool->pop();
	msg->msgType = msgType;
	msg->dataLen = dataLen;
	msg->isOk = isOk;

	if (dataLen > 0)
	{
		if (msg->data.size() - 1 < dataLen)
		{
			msg->data.resize(dataLen + 1);
		}
		memcpy(msg->data.data(), data, dataLen);
	}

	msg->source = serviceId;
	msg->session = session;
	serviceMgr->send(target, msg);

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

	lua_pushcfunction(state, destoryService);
	lua_setfield(state, -2, "destoryService");

	lua_pushcfunction(state, sendService);
	lua_setfield(state, -2, "send");

	if (!hasTable)
	{
		lua_setglobal(state, name);
	}
	else
	{
		lua_pop(state, 1);
	}
}