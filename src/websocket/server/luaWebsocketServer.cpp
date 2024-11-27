#include<lua/luaApi.hpp>
#include<websocket/server/websocketServerMgr.hpp>
#include<websocket/server/websocketServer.hpp>
#include <service/serviceMgr.hpp>
#include <service/msg/serviceMsgPool.hpp>

int32_t newWebsocketServer(lua_State* L)
{
	auto websocketMgr = WebsocketServerMgr::getInst();
	auto id = websocketMgr->newServer();

	lua_pushinteger(L, id);
	return 1;
}

int32_t destroyWebsocketServer(lua_State* L)
{
	auto id = luaL_checkinteger(L, 1);
	auto websocketMgr = WebsocketServerMgr::getInst();
	websocketMgr->destroyServer(id);
	return 0;
}

int32_t listenWebsocketServer(lua_State* L)
{
	auto id = luaL_checkinteger(L, 1);
	auto port = luaL_checkinteger(L, 2);
	const char* host = nullptr;
	if (lua_type(L, 3) == LUA_TSTRING)
	{
		host = lua_tostring(L, 3);
	}

	auto maxConnection = 0;
	if (lua_type(L, 4) == LUA_TNUMBER)
	{
		maxConnection = lua_tointeger(L, 4);
	}

	if (!host)
	{
		host = WebsocketDefaultHost;
	}

	if (maxConnection == 0)
	{
		maxConnection = WebsocketMaxConnection;
	}

	auto websocketMgr = WebsocketServerMgr::getInst();
	auto websocket = websocketMgr->getServer(id);
	if (!websocket)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	auto ret = websocket->listen(port, host, maxConnection);
	if (ret)
	{
		lua_getglobal(L, "SERVICE_ID");
		auto serviceId = luaL_checkinteger(L, -1);

		websocket->setOnConnectFunc([id,serviceId](uint32_t fd)
			{
				auto msg=ServiceMsgPool::getInst()->pop();
				msg->msgType = static_cast<uint32_t>(ServiceMsgType::WebsocketServer);
				msg->status = static_cast<uint32_t>(WebsocketMsgType::Open);
				msg->session = id;
				msg->fd = fd;
				ServiceMgr::getInst()->send(serviceId, msg);
			});

		websocket->setOnMsgFunc([id, serviceId](uint32_t fd,const std::string msgData)
			{
				auto msg = ServiceMsgPool::getInst()->pop();
				msg->msgType = static_cast<uint32_t>(ServiceMsgType::WebsocketServer);
				msg->status = static_cast<uint32_t>(WebsocketMsgType::Msg);
				msg->session = id;
				msg->fd = fd;
				auto len = msgData.length();
				if (msg->data.size() < len)
				{
					msg->data.resize(len + 1);
				}
				memcpy(msg->data.data(), msgData.c_str(), len);

				ServiceMgr::getInst()->send(serviceId, msg);
			});

		websocket->setOnCloseFunc([id, serviceId](uint32_t fd)
			{
				auto msg = ServiceMsgPool::getInst()->pop();
				msg->msgType = static_cast<uint32_t>(ServiceMsgType::WebsocketServer);
				msg->status = static_cast<uint32_t>(WebsocketMsgType::Close);
				msg->session = id;
				msg->fd = fd;
				ServiceMgr::getInst()->send(serviceId, msg);
			});
	}

	lua_pushboolean(L, ret);
	return 1;
}

int32_t sendWebsocketServer(lua_State* L)
{
	auto id = luaL_checkinteger(L, 1);
	auto fd = luaL_checkinteger(L, 2);
	auto data = luaL_checkstring(L, 3);
	auto dataLen = luaL_checkinteger(L, 4);

	auto websocketMgr = WebsocketServerMgr::getInst();
	auto server=websocketMgr->getServer(id);
	if (!server)
	{
		return 0;
	}

	server->send(fd, std::string(data, dataLen));

	return 0;
}

void luaRegisterWebsocketServerAPI(lua_State* state)
{
	lua_settop(state, 0);

	lua_newtable(state);

	lua_pushcfunction(state, newWebsocketServer);
	lua_setfield(state, -2, "new");

	lua_pushcfunction(state, destroyWebsocketServer);
	lua_setfield(state, -2, "destroy");

	lua_pushcfunction(state, listenWebsocketServer);
	lua_setfield(state, -2, "listen");

	lua_pushcfunction(state, sendWebsocketServer);
	lua_setfield(state, -2, "send");

	lua_setglobal(state, "websocketServer");
}