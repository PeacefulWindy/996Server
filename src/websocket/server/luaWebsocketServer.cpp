#include<lua/luaApi.hpp>
#include<websocket/server/websocketServerMgr.hpp>
#include<websocket/server/websocketServer.hpp>
#include <service/serviceMgr.hpp>
#include <service/msg/serviceMsgPool.hpp>
#include<cstring>

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

		websocket->setOnConnectFunc([id,serviceId](uint64_t fd)
			{
				auto msg=ServiceMsgPool::getInst()->pop();
				msg->msgType = static_cast<uint32_t>(ServiceMsgType::WebsocketServer);
				msg->status = static_cast<uint32_t>(WebsocketMsgType::Open);
				msg->session = id;
				msg->fd = fd;
				ServiceMgr::getInst()->send(serviceId, msg);
			});

		websocket->setOnMsgFunc([id, serviceId](uint64_t fd,const std::string &msgData)
			{
				auto msg = ServiceMsgPool::getInst()->pop();
				msg->msgType = static_cast<uint32_t>(ServiceMsgType::WebsocketServer);
				msg->status = static_cast<uint32_t>(WebsocketMsgType::Msg);
				msg->session = id;
				msg->fd = fd;
				msg->dataLen = msgData.length();
				if (msg->data.size() < msg->dataLen)
				{
					msg->data.resize(msg->dataLen + 1);
				}
				memcpy(msg->data.data(), msgData.c_str(), msg->dataLen);

				ServiceMgr::getInst()->send(serviceId, msg);
			});

		websocket->setOnCloseFunc([id, serviceId](uint64_t fd)
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
	auto dataLen = static_cast<size_t>(luaL_len(L, 3));
	auto data = luaL_checklstring(L, 3, &dataLen);

	auto websocketMgr = WebsocketServerMgr::getInst();
	auto server=websocketMgr->getServer(id);
	if (!server)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	auto ret=server->send(fd, std::string(data, dataLen));
	lua_pushboolean(L, ret);

	return 1;
}

int32_t closeWebsocketServer(lua_State* L)
{
	auto id = luaL_checkinteger(L, 1);
	auto fd = luaL_checkinteger(L, 2);

	auto websocketMgr = WebsocketServerMgr::getInst();
	auto server = websocketMgr->getServer(id);
	if (!server)
	{
		return 0;
	}

	server->close(fd);

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

	lua_pushcfunction(state, closeWebsocketServer);
	lua_setfield(state, -2, "close");

	lua_setglobal(state, "websocketServer");
}