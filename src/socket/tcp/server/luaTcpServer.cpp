#include<lua/luaApi.hpp>
#include <service/serviceMgr.hpp>
#include <service/msg/serviceMsgPool.hpp>
#include<socket/tcp/server/tcpServerMgr.hpp>
#include<socket/tcp/server/tcpServer.hpp>
#include<define.hpp>

int32_t newTcpServer(lua_State* L)
{
	auto tcpServerMgr = TcpServerMgr::getInst();
	auto id = tcpServerMgr->newServer();

	lua_pushinteger(L, id);
	return 1;
}

int32_t destroyTcpServer(lua_State* L)
{
	auto id = luaL_checkinteger(L, 1);
	auto tcpServerMgr = TcpServerMgr::getInst();
	tcpServerMgr->destroyServer(id);
	return 0;
}

int32_t listenTcpServer(lua_State* L)
{
	auto id = luaL_checkinteger(L, 1);
	auto port = luaL_checkinteger(L, 2);
	const char* host = nullptr;
	if (lua_type(L, 3) == LUA_TSTRING)
	{
		host = lua_tostring(L, 3);
	}

	if (!host)
	{
		host = TcpServerDefaultHost;
	}

	auto tcpServerMgr = TcpServerMgr::getInst();
	auto tcpServer = tcpServerMgr->getServer(id);
	if (!tcpServer)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	auto ret = tcpServer->listen(port, host);
	if (ret)
	{
		lua_getglobal(L, "SERVICE_ID");
		auto serviceId = luaL_checkinteger(L, -1);

		tcpServer->setOnConnectFunc([id, serviceId](uint64_t fd)
			{
				auto msg = ServiceMsgPool::getInst()->pop();
				msg->msgType = static_cast<uint32_t>(ServiceMsgType::TcpServer);
				msg->status = static_cast<uint32_t>(TcpMsgType::Connect);
				msg->session = id;
				msg->fd = fd;
				ServiceMgr::getInst()->send(serviceId, msg);
			});

		tcpServer->setOnCloseFunc([id, serviceId](uint64_t fd)
			{
				auto msg = ServiceMsgPool::getInst()->pop();
				msg->msgType = static_cast<uint32_t>(ServiceMsgType::TcpServer);
				msg->status = static_cast<uint32_t>(TcpMsgType::Close);
				msg->session = id;
				msg->fd = fd;
				ServiceMgr::getInst()->send(serviceId, msg);
			});

		tcpServer->setOnMsgFunc([id, serviceId](uint64_t fd, const std::string &msgData)
			{
				auto msg = ServiceMsgPool::getInst()->pop();
				msg->msgType = static_cast<uint32_t>(ServiceMsgType::TcpServer);
				msg->status = static_cast<uint32_t>(TcpMsgType::Msg);
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
	}

	lua_pushboolean(L, ret);
	return 1;
}

int32_t sendTcpServer(lua_State* L)
{
	auto id = luaL_checkinteger(L, 1);
	auto fd = luaL_checkinteger(L, 2);
	auto dataLen = static_cast<size_t>(luaL_len(L, 3));
	auto data = luaL_checklstring(L, 3, &dataLen);

	auto tcpServerMgr = TcpServerMgr::getInst();
	auto server = tcpServerMgr->getServer(id);
	if (!server)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	auto ret=server->send(fd, std::string(data, dataLen));
	lua_pushboolean(L, ret);

	return 1;
}

int32_t closeTcpServer(lua_State* L)
{
	auto id = luaL_checkinteger(L, 1);
	auto fd = luaL_checkinteger(L, 2);

	auto tcpServerMgr = TcpServerMgr::getInst();
	auto server = tcpServerMgr->getServer(id);
	if (!server)
	{
		return 0;
	}

	server->close(fd);

	return 0;
}

int32_t getRemoteInfoTcpServer(lua_State* L)
{
	auto id = luaL_checkinteger(L, 1);
	auto fd = luaL_checkinteger(L, 2);

	auto tcpServerMgr = TcpServerMgr::getInst();
	auto server = tcpServerMgr->getServer(id);
	if (!server)
	{
		return 0;
	}

	auto remoteInfo=server->getRemoteInfo(fd);

	lua_newtable(L);

	lua_pushstring(L, remoteInfo->host.c_str());
	lua_setfield(L, -2, "host");

	lua_pushinteger(L, remoteInfo->port);
	lua_setfield(L, -2, "port");

	return 1;
}

void luaRegisterTcpServerAPI(lua_State* state)
{
	lua_settop(state, 0);

	lua_newtable(state);

	lua_pushcfunction(state, newTcpServer);
	lua_setfield(state, -2, "new");

	lua_pushcfunction(state, destroyTcpServer);
	lua_setfield(state, -2, "destroy");

	lua_pushcfunction(state, listenTcpServer);
	lua_setfield(state, -2, "listen");

	lua_pushcfunction(state, sendTcpServer);
	lua_setfield(state, -2, "send");

	lua_pushcfunction(state, closeTcpServer);
	lua_setfield(state, -2, "close");

	lua_pushcfunction(state, getRemoteInfoTcpServer);
	lua_setfield(state, -2, "getRemoteInfo");

	lua_setglobal(state, "tcpServer");
}