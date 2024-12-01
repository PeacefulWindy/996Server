#include<lua/luaApi.hpp>
#include<iostream>
#include<spdlog/spdlog.h>
#include <worker/workerMgr.hpp>
#include <stdlib.h>
#include <service/serviceMgr.hpp>
#include <ixwebsocket/IXNetSystem.h>
#include <service/msg/serviceMsgPool.hpp>
#include<asio.hpp>
#include<filesystem>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

struct ServerConfig
{
	int32_t threadNum = std::thread::hardware_concurrency();
	std::string startScript;
	std::string logFilePath;
#ifdef _DEBUG
	std::string logLevel = "debug";
#else
	std::string logLevel = "info";
#endif
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
		auto key = std::string(luaL_checkstring(L, -2));
		if (key == "thread")
		{
			config.threadNum = luaL_checknumber(L, -1);
		}
		else if (key == "start")
		{
			config.startScript = luaL_checkstring(L, -1);
		}
		else if(key == "logFile")
		{
			config.logFilePath = luaL_checkstring(L, -1);
		}
		else if (key == "luaPath")
		{
			config.luaPath = luaL_checkstring(L, -1);
		}
		else if (key == "cPath")
		{
			config.cPath = luaL_checkstring(L, -1);
		}
		else if (key == "servicePath")
		{
			auto argIndex = lua_gettop(L);
			lua_pushnil(L);
			while (lua_next(L, argIndex) != 0)
			{
				config.servicePaths.push_back(luaL_checkstring(L, -1));
				lua_pop(L, 1);
			}
		}
		else if (key == "args")
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

std::shared_ptr<spdlog::logger> Logger;

bool setLog(ServerConfig& config)
{
	auto logLevel = spdlog::level::debug;

	if (config.logLevel == "info")
	{
		logLevel = spdlog::level::info;
	}
	else if (config.logLevel == "error")
	{
		logLevel = spdlog::level::err;
	}
	else if (config.logLevel == "warn")
	{
		logLevel = spdlog::level::warn;
	}

	auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	consoleSink->set_level(logLevel);

	auto fileSink = std::shared_ptr<spdlog::sinks::basic_file_sink_mt>();

	if (!config.logFilePath.empty())
	{
		auto path = std::filesystem::u8path(config.logFilePath);
		auto dirPath = path.parent_path();
		if (!std::filesystem::exists(dirPath))
		{
			std::filesystem::create_directories(dirPath);
		}

		try
		{
			fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(config.logFilePath, true);
			fileSink->set_level(logLevel);
		}
		catch (const spdlog::spdlog_ex& e)
		{
			spdlog::error(e.what());
			return false;
		}
	}

	auto sinks = std::vector<spdlog::sink_ptr>();
	sinks.emplace_back(consoleSink);
	if (fileSink)
	{
		sinks.emplace_back(fileSink);
	}

	Logger = std::make_shared<spdlog::logger>("log", sinks.begin(), sinks.end());
	Logger->set_level(logLevel);
	spdlog::set_default_logger(Logger);
	spdlog::flush_on(logLevel);
	spdlog::flush_every(std::chrono::seconds(5));

	return true;
}

asio::io_context IoContext;

int main(int arg,char * argv[])
{
	ServiceMsgPool::newInst(10);

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
	luaL_openlibs(L);

	if (luaL_loadfile(L, configPath) != LUA_OK)
	{
		spdlog::error("not found config:{},exit", configPath);
		return -1;
	}

	auto argNum = arg - 2;
	for (auto i = 2; i < arg; i++)
	{
		lua_pushstring(L, argv[i]);
	}

	if (lua_pcall(L, argNum, LUA_MULTRET, 0) != LUA_OK)
	{
		spdlog::error("{}", lua_tostring(L, -1));
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
	if (!setLog(config))
	{
		return -1;
	}

	L = luaNewState();

	if (luaL_loadfile(L, config.startScript.c_str()) != LUA_OK)
	{
		spdlog::error("not found start script:{},exit", config.startScript);
		return -1;
	}

	auto serviceMgr = ServiceMgr::getInst();
	for (auto it = config.servicePaths.begin(); it != config.servicePaths.end(); ++it)
	{
		serviceMgr->addServicePath(*it);
	}

	auto workerMgr = WorkerMgr::getInst();
	workerMgr->initWorker(config.threadNum);

	auto configArgNum = config.args.size();
	for (auto i = 0;i< configArgNum; i++)
	{
		auto &arg = config.args[i];
		lua_pushstring(L, arg.c_str());
	}

	if (lua_pcall(L, configArgNum, LUA_MULTRET, 0) != LUA_OK)
	{
		spdlog::error("{}", lua_tostring(L, -1));
		workerMgr->destroy();
		return -1;
	}

	while (luaIsRun())
	{
		IoContext.poll();
		serviceMgr->poll();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	ServiceMgr::destroy();
	WorkerMgr::destroy();

	ix::uninitNetSystem();

	return 0;
}