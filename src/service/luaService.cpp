#include<lua/luaApi.hpp>
#include<service/serviceMgr.hpp>
#include<service/service.hpp>
#include<service/msg/serviceMsgPool.hpp>

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
	auto data = luaL_checkstring(L, 4);
	auto dataLen = luaL_checkinteger(L, 5);

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

int getServiceAllMsg(lua_State* L)
{
	lua_getglobal(L, "SERVICE_PTR");
	auto service = static_cast<Service*>(lua_touserdata(L, 1));
	auto msgs=service->popAllMsg();
	lua_settop(L, 0);

	auto serviceMsgPool = ServiceMsgPool::getInst();

	lua_newtable(L);
	for (auto i = 0; i < msgs.size(); i++)
	{
		auto& it = msgs.at(i);
		lua_newtable(L);

		lua_pushinteger(L, it->msgType);
		lua_setfield(L, -2, "msgType");

		lua_pushinteger(L, it->session);
		lua_setfield(L, -2, "session");

		lua_pushinteger(L, it->source);
		lua_setfield(L, -2, "source");

		lua_pushstring(L, reinterpret_cast<const char*>(it->data.data()));
		lua_setfield(L, -2, "data");

		lua_pushstring(L, it->error.c_str());
		lua_setfield(L, -2, "error");

		lua_pushinteger(L, it->status);
		lua_setfield(L, -2, "status");

		lua_pushinteger(L, it->fd);
		lua_setfield(L, -2, "fd");

		lua_rawseti(L, -2, static_cast<lua_Integer>(i) + 1);

		serviceMsgPool->push(it);
	}

	return 1;
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

	lua_pushcfunction(state, getServiceAllMsg);
	lua_setfield(state, -2, "getAllMsg");

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