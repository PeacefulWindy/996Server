#include "service.hpp"
#include<spdlog/spdlog.h>
#include<service/serviceMgr.hpp>
#include<filesystem>
#include <worker/workerMgr.hpp>

Service::Service(int32_t id, std::string name, std::string src)
	:mId(id),mName(name)
{
	L = luaNewState();

	auto& basePaths = ServiceMgr::getInst()->getServicePath();
	auto filePath = std::string();
	for (auto it = basePaths.begin(); it != basePaths.end(); ++it)
	{
		auto srcPath = (*it) + "/" + src;
		if (std::filesystem::exists(srcPath))
		{
			filePath = srcPath;
			break;
		}
	}

	if (filePath.size() == 0)
	{
		spdlog::error("not found service file:{}", src.c_str());
		luaExit();
		return;
	}

	if (luaL_dofile(L, filePath.c_str()) != LUA_OK)
	{
		spdlog::error("not found service file:{}", filePath.c_str());
		luaExit();
		return;
	}

	spdlog::info("service [{}]:{} start.", this->mId, this->mName);

	this->mIsInit = true;
}

Service::~Service()
{
	if (this->mIsInit)
	{
		lua_getglobal(L, "api");
	}
	
	lua_close(L);

	spdlog::info("service [{}]:{} stop.", this->mId, this->mName);
}

const std::string Service::getName() const
{
	return this->mName;
}

int32_t Service::getId() const
{
	return this->mId;
}

void Service::poll()
{

}

