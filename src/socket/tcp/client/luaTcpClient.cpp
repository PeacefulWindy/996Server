#include<lua/luaApi.hpp>
#include <service/serviceMgr.hpp>
#include <service/msg/serviceMsgPool.hpp>
#include<socket/tcp/Client/tcpClientMgr.hpp>
#include<socket/tcp/Client/tcpClient.hpp>
#include<define.hpp>

int32_t newTcpClient(lua_State* L)
{
	auto tcpClientMgr = TcpClientMgr::getInst();
	auto id = tcpClientMgr->newClient();

	lua_pushinteger(L, id);
	return 1;
}

int32_t destroyTcpClient(lua_State* L)
{
	auto id = luaL_checkinteger(L, 1);
	auto tcpClientMgr = TcpClientMgr::getInst();
	tcpClientMgr->destroyClient(id);
	return 0;
}

int32_t connectTcpClient(lua_State* L)
{
	auto id = luaL_checkinteger(L, 1);
	auto host = lua_tostring(L, 2);
	auto port = luaL_checkinteger(L, 3);

	auto tcpClientMgr = TcpClientMgr::getInst();
	auto tcpClient = tcpClientMgr->getClient(id);
	if (!tcpClient)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	auto ret = tcpClient->connect(host, port);
	if (ret)
	{
		lua_getglobal(L, "SERVICE_ID");
		auto serviceId = luaL_checkinteger(L, -1);

		tcpClient->setOnConnectFunc([id, serviceId]()
			{
				auto msg = ServiceMsgPool::getInst()->pop();
				msg->msgType = static_cast<uint32_t>(ServiceMsgType::TcpClient);
				msg->status = static_cast<uint32_t>(TcpMsgType::Open);
				msg->session = id;
				ServiceMgr::getInst()->send(serviceId, msg);
			});

		tcpClient->setOnMsgFunc([id, serviceId](const std::string msgData)
			{
				auto msg = ServiceMsgPool::getInst()->pop();
				msg->msgType = static_cast<uint32_t>(ServiceMsgType::TcpClient);
				msg->status = static_cast<uint32_t>(TcpMsgType::Msg);
				msg->session = id;
				auto len = msgData.length();
				if (msg->data.size() < len)
				{
					msg->data.resize(len + 1);
				}
				memcpy(msg->data.data(), msgData.c_str(), len);

				ServiceMgr::getInst()->send(serviceId, msg);
			});

		tcpClient->setOnCloseFunc([id, serviceId]()
			{
				auto msg = ServiceMsgPool::getInst()->pop();
				msg->msgType = static_cast<uint32_t>(ServiceMsgType::TcpClient);
				msg->status = static_cast<uint32_t>(TcpMsgType::Close);
				msg->session = id;

				ServiceMgr::getInst()->send(serviceId, msg);
			});
	}

	lua_pushboolean(L, ret);
	return 1;
}

int32_t closeTcpClient(lua_State* L)
{
	auto id = luaL_checkinteger(L, 1);

	auto tcpClientMgr = TcpClientMgr::getInst();
	auto tcpClient = tcpClientMgr->getClient(id);
	if (!tcpClient)
	{
		return 0;
	}

	tcpClient->close();
	return 0;
}

int32_t sendTcpClient(lua_State* L)
{
	auto id = luaL_checkinteger(L, 1);
	auto data = luaL_checkstring(L, 2);
	auto dataLen = luaL_checkinteger(L, 3);

	auto tcpClientMgr = TcpClientMgr::getInst();
	auto tcpClient = tcpClientMgr->getClient(id);
	if (!tcpClient)
	{
		return 0;
	}

	tcpClient->send(std::string(data, dataLen));

	return 0;
}

void luaRegisterTcpClientAPI(lua_State* state)
{
	lua_settop(state, 0);

	lua_newtable(state);

	lua_pushcfunction(state, newTcpClient);
	lua_setfield(state, -2, "new");

	lua_pushcfunction(state, destroyTcpClient);
	lua_setfield(state, -2, "destroy");

	lua_pushcfunction(state, connectTcpClient);
	lua_setfield(state, -2, "connect");

	lua_pushcfunction(state, closeTcpClient);
	lua_setfield(state, -2, "close");

	lua_pushcfunction(state, sendTcpClient);
	lua_setfield(state, -2, "send");

	lua_setglobal(state, "tcpClient");
}