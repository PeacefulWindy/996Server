#include<lua/luaApi.hpp>
#include<iostream>
#include<spdlog/spdlog.h>
#include <worker/workerMgr.hpp>
#include <stdlib.h>
#include <service/serviceMgr.hpp>
#include <ixwebsocket/IXNetSystem.h>

struct ServerConfig
{
	int32_t threadNum = std::thread::hardware_concurrency();
	std::string startScript;
	std::string logFilePath;
	std::string logLevel = "debug";
	std::string cPath;
	std::string luaPath;
	std::vector<std::string> servicePaths;
	std::vector<std::string> args;
};

void printHelp()
{
	std::cout << "996server configPath args..." << std::endl;
	std::cout << "example:" << std::endl;
	std::cout << "996server config.lua arg1 arg2 ..." << std::endl;
}

bool readConfig(lua_State * L,ServerConfig &config)
{
	lua_getglobal(L,"Config");
	auto index = lua_gettop(L);
	if (lua_type(L, index) != LUA_TTABLE)
	{
		return false;
	}

	lua_pushnil(L);
	while (lua_next(L, index) != 0)
	{
		auto key = luaL_checkstring(L, -2);
		if (strcmp(key,"thread") == 0)
		{
			config.threadNum = luaL_checknumber(L, -1);
		}
		else if (strcmp(key, "start") == 0)
		{
			config.startScript = luaL_checkstring(L, -1);
		}
		else if(strcmp(key,"logFile") == 0)
		{
			config.logFilePath = luaL_checkstring(L, -1);
		}
		else if (strcmp(key, "luaPath") == 0)
		{
			config.luaPath = luaL_checkstring(L, -1);
		}
		else if (strcmp(key, "cPath") == 0)
		{
			config.cPath = luaL_checkstring(L, -1);
		}
		else if (strcmp(key, "servicePath") == 0)
		{
			auto argIndex = lua_gettop(L);
			lua_pushnil(L);
			while (lua_next(L, argIndex) != 0)
			{
				config.servicePaths.push_back(luaL_checkstring(L, -1));
				lua_pop(L, 1);
			}
		}
		else if (strcmp(key, "args") == 0)
		{
			auto argIndex = lua_gettop(L);
			lua_pushnil(L);
			while (lua_next(L, argIndex) != 0)
			{
				config.args.push_back(luaL_checkstring(L, -1));
				lua_pop(L, 1);
			}
		}

		lua_pop(L, 1);
	}

	if (config.startScript == "")
	{
		spdlog::error("invalid start script!");
		return false;
	}

	if (config.threadNum <= 0)
	{
		spdlog::error("invalid thread num!");
		return false;
	}

	return true;
}

int main(int arg,char * argv[])
{
	ix::initNetSystem();
	if (arg < 2)
	{
		printHelp();
		return 0;
	}

#if _WIN32
	system("chcp 65001");
	SetConsoleCtrlHandler([](DWORD signal)
		{
			if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT)
			{
				std::cout << "Caught Ctrl+C or Console Close Event!" << std::endl;
				luaExit();
				return TRUE;
			}

			return FALSE;
		}, true);
#else

#endif

	auto configPath = argv[1];

	auto L = luaL_newstate();

	if (luaL_dofile(L, configPath) != LUA_OK)
	{
		spdlog::error("not found config:{},exit", configPath);
		return -1;
	}

	auto config = ServerConfig();
	auto isReadConfig = readConfig(L, config);
	lua_close(L);

	if (!isReadConfig)
	{
		spdlog::error("invalid config:{},exit", configPath);
		return -1;
	}

	luaSetLuaPath(config.luaPath);
	luaSetCPath(config.cPath);

	auto serviceMgr = ServiceMgr::getInst();
	for (auto it = config.servicePaths.begin(); it != config.servicePaths.end(); ++it)
	{
		serviceMgr->addServicePath(*it);
	}

	auto workerMgr = WorkerMgr::getInst();
	workerMgr->initWorker(config.threadNum);

	L = luaNewState();

	lua_newtable(L);
	for (auto i = 0;i< config.args.size(); i++)
	{
		auto &arg = config.args[i];
		lua_pushstring(L, arg.c_str());
		lua_rawseti(L, -2, static_cast<lua_Integer>(i) + 1);
	}
	lua_setglobal(L, "SysArgs");

	if (luaL_dofile(L, config.startScript.c_str()) != LUA_OK)
	{
		spdlog::error("{}", lua_tostring(L, -1));
		workerMgr->destroy();
		return -1;
	}

	while (luaIsRun())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	ServiceMgr::destroy();
	WorkerMgr::destroy();

	ix::uninitNetSystem();

	return 0;
}