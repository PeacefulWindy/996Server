#include "service.hpp"
#include<spdlog/spdlog.h>
#include<service/serviceMgr.hpp>
#include<filesystem>
#include <worker/workerMgr.hpp>

Service::Service(int32_t id, std::string name, std::string src)
	:mId(id),mName(name)
{
	this->mState = luaNewState();

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

	if (luaL_dofile(this->mState, filePath.c_str()) != LUA_OK)
	{
		spdlog::error("{}", lua_tostring(this->mState,-1));
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
		lua_settop(this->mState, 0);
		lua_getglobal(this->mState, "api");
	}
	
	lua_close(this->mState);

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
	if (!this->mIsInit)
	{
		return;
	}

	lua_settop(this->mState, 0);
	lua_getglobal(this->mState,"onPoll");

	if (lua_isfunction(this->mState, -1))
	{
		if (lua_pcall(this->mState, 0, 0, -1) != LUA_OK)
		{
			spdlog::error("{}", lua_tostring(this->mState, -1));
		}
	}
}

