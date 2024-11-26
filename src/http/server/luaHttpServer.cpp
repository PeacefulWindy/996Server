#include<lua/luaApi.hpp>
#include<http/server/httpServerMgr.hpp>
#include<http/server/httpSever.hpp>
#include<spdlog/spdlog.h>

int32_t newHttpServer(lua_State* L)
{
	if (!lua_isfunction(L, 1))
	{
		lua_pushinteger(L, 0);
		return 1;
	}

	auto refFuncId = luaL_ref(L, LUA_REGISTRYINDEX);

	auto httpServerMgr = HttpServerMgr::getInst();
	auto id = httpServerMgr->newServer();
	auto httpServer = httpServerMgr->getServer(id);

	httpServer->setOnMsgFunc([L,refFuncId](std::shared_ptr<HttpRequest> request)
		{
			auto response = std::make_shared<HttpResponse>();

			lua_rawgeti(L, LUA_REGISTRYINDEX, refFuncId);
			lua_newtable(L);

			lua_pushstring(L, request->method.c_str());
			lua_setfield(L, -2, "method");

			lua_pushstring(L, request->version.c_str());
			lua_setfield(L, -2, "version");

			lua_pushstring(L, request->body.c_str());
			lua_setfield(L, -2, "body");

			lua_newtable(L);
			for (auto it = request->headers.begin(); it != request->headers.end(); ++it)
			{
				lua_pushstring(L, it->second.c_str());
				lua_setfield(L, -2, it->first.c_str());
			}

			lua_setfield(L, -2, "headers");

			if (lua_pcall(L, 1, 1, 0) != LUA_OK)
			{
				response->status = 500;
				spdlog::error("{}", lua_tostring(L, -1));
				return response;
			}

			if (!lua_istable(L, 1))
			{
				response->status = 404;
				return response;
			}

			lua_pushnil(L);
			while (lua_next(L, 1))
			{
				auto key=lua_tostring(L, -2);
				if (strcmp(key, "msg") == 0)
				{
					response->msg = lua_tostring(L, -1);
				}
				else if (strcmp(key, "status") == 0)
				{
					response->status = lua_tointeger(L, -1);
				}

				lua_pop(L, 1);
			}

			return response;
		});

	lua_pushinteger(L, id);
	return 1;
}

int32_t destroyHttpServer(lua_State* L)
{
	auto id = luaL_checkinteger(L, 1);
	auto httpServerMgr = HttpServerMgr::getInst();
	httpServerMgr->destroyServer(id);
	return 0;
}

int32_t listenHttpServer(lua_State* L)
{
	auto id = luaL_checkinteger(L, 1);
	auto port = luaL_checkinteger(L, 2);
	auto httpServerMgr = HttpServerMgr::getInst();
	auto httpServer=httpServerMgr->getServer(id);
	if (!httpServer)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	auto ret = httpServer->listen(port);
	lua_pushboolean(L, ret);
	return 1;
}

void luaRegisterHttpServerAPI(lua_State* state)
{
	lua_settop(state, 0);

	lua_newtable(state);

	lua_pushcfunction(state, newHttpServer);
	lua_setfield(state, -2, "new");

	lua_pushcfunction(state, destroyHttpServer);
	lua_setfield(state, -2, "destroy");

	lua_pushcfunction(state, listenHttpServer);
	lua_setfield(state, -2, "listen");

	//lua_pushcfunction(state, destroyService);
	//lua_setfield(state, -2, "destroyService");

	//lua_pushcfunction(state, getServiceAllMsg);
	//lua_setfield(state, -2, "getAllMsg");

	//lua_pushcfunction(state, sendService);
	//lua_setfield(state, -2, "send");

	lua_setglobal(state, "httpServer");
}