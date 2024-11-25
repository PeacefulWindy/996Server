#include<lua/luaApi.hpp>
#include<service/serviceMgr.hpp>
#include<service/service.hpp>

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

int sendService(lua_State* L)
{
	auto target = luaL_checkinteger(L, 1);
	auto msgType = luaL_checkinteger(L, 2);
	auto session= luaL_checkinteger(L, 3);
	auto args = luaL_checkstring(L, 4);

	lua_getglobal(L, "SERVICE_ID");
	auto serviceId = luaL_checkinteger(L, -1);

	if (msgType <= ServiceMsgType::None)
	{
		return 0;
	}

	auto serviceMgr = ServiceMgr::getInst();
	auto msg = std::make_shared<ServiceMsg>();
	msg->msgType = msgType;
	msg->args = args;
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

	lua_newtable(L);
	for (auto i = 0; i < msgs.size(); i++)
	{
		auto& it = msgs.at(i);
		lua_newtable(L);

		lua_pushinteger(L, it->msgType);
		lua_setfield(L, -2, "msgType");

		lua_pushstring(L, it->args.c_str());
		lua_setfield(L, -2, "args");

		lua_pushinteger(L, it->session);
		lua_setfield(L, -2, "session");

		lua_pushinteger(L, it->source);
		lua_setfield(L, -2, "source");

		lua_rawseti(L, -2, static_cast<lua_Integer>(i) + 1);
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

	lua_pushcfunction(state, destroyService);
	lua_setfield(state, -2, "destroyService");

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