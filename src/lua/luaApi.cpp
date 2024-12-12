#include"lua.hpp"
#include "luaApi.hpp"
#include<string>
#include <chrono>
#include<asio/steady_timer.hpp>
#include<service/msg/serviceMsg.hpp>
#include<service/msg/serviceMsgPool.hpp>
#include<service/serviceMgr.hpp>
#include<thread>
#ifdef _WIN32
#include <conio.h>
#endif

extern asio::io_context IoContext;

bool IsRun = true;
std::mutex EnvLock;
std::unordered_map<std::string, std::string> Envs;
std::string CPath;
std::string LuaPath;

extern void luaRegisterFsAPI(lua_State* state);
extern void luaRegisterConfigAPI(lua_State* state);
extern void luaRegisterProtobufAPI(lua_State* state);
extern void luaRegisterJsonAPI(lua_State* state);
extern void luaRegisterMariadbAPI(lua_State* state);
extern void luaRegisterLogAPI(lua_State* state);
extern void luaRegisterServiceAPI(lua_State* state);
extern void luaRegisterHttpClientAPI(lua_State* state);
extern void luaRegisterWebsocketServerAPI(lua_State* state);
extern void luaRegisterTcpServerAPI(lua_State* state);
extern void luaRegisterTcpClientAPI(lua_State* state);
extern void luaRegisterRedisAPI(lua_State* state);
extern void luaRegisterCryptoBase64API(lua_State* state);
extern void luaRegisterCryptoRsaAPI(lua_State* state);
extern void luaRegisterCryptoShaAPI(lua_State* state);
extern void luaRegisterUUIDAPI(lua_State* state);

int exit(lua_State* L)
{
	IsRun = false;
	return 0;
}

void luaExit()
{
	IsRun = false;
}

int luaGetChar(lua_State* L)
{
#ifdef _WIN32
	if (kbhit())
	{
		lua_pushinteger(L, getche());
		return 1;
	}
#else
//not test
	auto rfds = fd_set();
	auto tv = timeval();
	system(STTY_US TTY_PATH);
	FD_ZERO(&rfds);
	FD_SET(0, &rfds);
	tv.tv_sec = 0;
	tv.tv_usec = 10;
	if (select(1, &rfds, NULL, NULL, &tv) > 0)
	{
		lua_pushinteger(L, getchar());
		return 1;
	}
#endif
	return 0;
}

void luaRegisterCoreAPI(lua_State* state)
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

	lua_pushcfunction(state, exit);
	lua_setfield(state, -2, "exit");

	lua_pushcfunction(state, [](lua_State* L)->int32_t
		{
			auto key = luaL_checkstring(L, 1);
			auto value = luaL_checkstring(L, 2);

			while (!EnvLock.try_lock())
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}

			Envs.insert({ key,value });

			EnvLock.unlock();
			return 1;
		});
	lua_setfield(state, -2, "setEnv");

	lua_pushcfunction(state, [](lua_State* L)->int32_t
		{
			auto key = luaL_checkstring(L, 1);

			while (!EnvLock.try_lock())
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}

			auto iter = Envs.find(key);
			if (iter != Envs.end())
			{
				lua_pushstring(L, iter->second.c_str());
			}

			EnvLock.unlock();
			return 1;
		});
	lua_setfield(state, -2, "env");

	lua_pushcfunction(state, [](lua_State* L)->int32_t
		{
			auto now = std::chrono::system_clock::now();
			auto second = std::chrono::time_point_cast<std::chrono::seconds>(now);
			lua_pushinteger(L, second.time_since_epoch().count());
			return 1;
		});
	lua_setfield(state, -2, "time");

	lua_pushcfunction(state, [](lua_State* L)->int32_t
		{
			auto now = std::chrono::system_clock::now();
			auto milliseconds = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
			lua_pushinteger(L, milliseconds.time_since_epoch().count());
			return 1;
		});
	lua_setfield(state, -2, "mstime");

	lua_pushcfunction(state, [](lua_State* L)->int32_t
		{
			auto session = luaL_checkinteger(L, 1);
			auto millsecond = luaL_checkinteger(L, 2);

			lua_getglobal(L, "SERVICE_ID");
			auto serviceId = luaL_checkinteger(L, -1);

			auto timer=std::make_shared<asio::steady_timer>(IoContext, std::chrono::milliseconds(millsecond));
			timer->async_wait([timer, session, serviceId](std::error_code ec)->void
				{
					if (ec)
					{
						return;
					}

					auto serviceMsgPool = ServiceMsgPool::getInst();
					auto msg=serviceMsgPool->pop();
					msg->session = session;
					msg->msgType = static_cast<uint32_t>(ServiceMsgType::Timer);

					ServiceMgr::getInst()->send(serviceId, msg);
				});

			return 0;
		});
	lua_setfield(state, -2, "timer");

	lua_pushcfunction(state, luaGetChar);
	lua_setfield(state, -2, "getInputChar");

	lua_pushcfunction(state, [](lua_State* L)->int32_t
		{
			auto text = luaL_checkstring(L, 1);
			printf(text);
			return 0;
		});
	lua_setfield(state, -2, "print");

	if (!hasTable)
	{
		lua_setglobal(state, name);
	}
	else
	{
		lua_pop(state, 1);
	}
}

void luaRegisterAPI(lua_State* state)
{
#ifdef _DEBUG
	lua_pushboolean(state, true);
#else
	lua_pushboolean(state, false);
#endif
	lua_setglobal(state, "DEBUG_MODE");

	luaRegisterCoreAPI(state);
	luaRegisterServiceAPI(state);
	luaRegisterFsAPI(state);
	luaRegisterConfigAPI(state);
	luaRegisterLogAPI(state);
	luaRegisterProtobufAPI(state);
	luaRegisterJsonAPI(state);
	luaRegisterMariadbAPI(state);
	luaRegisterHttpClientAPI(state);
	luaRegisterWebsocketServerAPI(state);
	luaRegisterTcpServerAPI(state);
	luaRegisterTcpClientAPI(state);
	luaRegisterRedisAPI(state);
	luaRegisterCryptoBase64API(state);
	luaRegisterCryptoRsaAPI(state);
	luaRegisterCryptoShaAPI(state);
	luaRegisterUUIDAPI(state);
}

void luaSetCPath(std::string path)
{
	CPath = path;
}

void luaSetLuaPath(std::string path)
{
	LuaPath = path;
}

bool luaIsRun()
{
	return IsRun;
}

lua_State* luaNewState(std::string externLuaPath)
{
	auto L = luaL_newstate();
	luaL_openlibs(L);

	lua_getglobal(L, "package");
	
	auto path = LuaPath;
	if (externLuaPath.length() > 0)
	{
		path = externLuaPath + ";" + path;
	}

	lua_pushstring(L, path.c_str());
	lua_setfield(L, -2, "path");

	lua_pushstring(L, CPath.c_str());
	lua_setfield(L, -2, "cpath");

	lua_pop(L, 1);

	luaRegisterAPI(L);

	return L;
}

void luaPrintStack(lua_State* state)
{
	printf("==============Lua Stack ==============\n");
	auto size = lua_gettop(state);
	for (auto i = size; i > 0; i--)
	{
		auto typeId = lua_type(state, i);
		auto typeName = lua_typename(state, typeId);
		auto valueStr = std::string();
		switch (typeId)
		{
		case LUA_TNONE:
			valueStr = "none";
			break;
		case LUA_TNIL:
			valueStr = "nil";
			break;
		case LUA_TBOOLEAN:
			valueStr = std::to_string(lua_toboolean(state, i));
			break;
		case LUA_TLIGHTUSERDATA:
			valueStr = std::to_string((int64_t)lua_touserdata(state, i));
			break;
		case LUA_TNUMBER:
			valueStr = std::to_string(lua_tonumber(state, i));
			break;
		case LUA_TSTRING:
			valueStr = std::string(lua_tostring(state, i));
			break;
		case LUA_TTABLE:
			valueStr = "table";
			break;
		case LUA_TFUNCTION:
			valueStr = "function";
			break;
		case LUA_TUSERDATA:
			valueStr = std::to_string((int64_t)lua_touserdata(state, i));
			break;
		case LUA_TTHREAD:
			valueStr = "coroutine";
			break;
		}

		printf("%d %d (%s)%s\n", i - size - 1, i, typeName, valueStr.c_str());
	}
	printf("==============Lua Stack End==============\n\n");
}