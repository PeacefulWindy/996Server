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

		tcpServer->setOnConnectFunc([id, serviceId](uint32_t fd)
			{
				auto msg = ServiceMsgPool::getInst()->pop();
				msg->msgType = static_cast<uint32_t>(ServiceMsgType::TcpServer);
				msg->status = static_cast<uint32_t>(TcpMsgType::Open);
				msg->session = id;
				msg->fd = fd;
				ServiceMgr::getInst()->send(serviceId, msg);
			});

		tcpServer->setOnCloseFunc([id, serviceId](uint32_t fd)
			{
				auto msg = ServiceMsgPool::getInst()->pop();
				msg->msgType = static_cast<uint32_t>(ServiceMsgType::TcpServer);
				msg->status = static_cast<uint32_t>(TcpMsgType::Close);
				msg->session = id;
				msg->fd = fd;
				ServiceMgr::getInst()->send(serviceId, msg);
			});

		tcpServer->setOnMsgFunc([id, serviceId](uint32_t fd, const std::string msgData)
			{
				auto msg = ServiceMsgPool::getInst()->pop();
				msg->msgType = static_cast<uint32_t>(ServiceMsgType::TcpServer);
				msg->status = static_cast<uint32_t>(TcpMsgType::Msg);
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
	}

	lua_pushboolean(L, ret);
	return 1;
}

int32_t sendTcpServer(lua_State* L)
{
	auto id = luaL_checkinteger(L, 1);
	auto fd = luaL_checkinteger(L, 2);
	auto data = luaL_checkstring(L, 3);
	auto dataLen = luaL_checkinteger(L, 4);

	auto tcpServerMgr = TcpServerMgr::getInst();
	auto server = tcpServerMgr->getServer(id);
	if (!server)
	{
		return 0;
	}

	server->send(fd, std::string(data, dataLen));

	return 0;
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

	lua_setglobal(state, "tcpServer");
}