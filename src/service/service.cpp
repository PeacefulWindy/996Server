#include "service.hpp"
#include<spdlog/spdlog.h>
#include<service/serviceMgr.hpp>
#include<filesystem>
#include <worker/workerMgr.hpp>
#include<service/msg/serviceMsgPool.hpp>

Service::Service(int32_t id, std::string name, std::string src,std::string args)
	:mId(id),mName(name)
{
	auto& basePaths = ServiceMgr::getInst()->getServicePath();
	auto rootPath = std::string();
	auto filePath = std::string();
	for (auto it = basePaths.begin(); it != basePaths.end(); ++it)
	{
		auto srcPath = (*it) + "/" + src;
		if (std::filesystem::exists(srcPath))
		{
			rootPath = *it;
			filePath = srcPath;
			break;
		}
	}

	if (filePath.length() == 0)
	{
		spdlog::error("not found service file:{}", src.c_str());
		luaExit();
		return;
	}

	this->mState = luaNewState(rootPath + "/?.lua");

	lua_pushinteger(this->mState, this->mId);
	lua_setglobal(this->mState, "SERVICE_ID");

	lua_pushstring(this->mState, this->mName.c_str());
	lua_setglobal(this->mState, "SERVICE_NAME");

	lua_pushstring(this->mState, args.c_str());
	lua_setglobal(this->mState, "SERVICE_SYSARGS");

	lua_pushlightuserdata(this->mState, this);
	lua_setglobal(this->mState, "SERVICE_PTR");

	if (luaL_dofile(this->mState, filePath.c_str()) != LUA_OK)
	{
		spdlog::error("{}", lua_tostring(this->mState,-1));
		luaExit();
		return;
	}

	spdlog::info("service_{}:{} start.", this->mId, this->mName);

	this->mIsInit = true;
}

Service::~Service()
{
	lua_close(this->mState);

	spdlog::info("service_{}:{} stop.", this->mId, this->mName);
}

void Service::close()
{
	if (!this->mIsInit)
	{
		return;
	}

	while (!this->mMsgLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	this->mIsInit = false;

	this->mMsgLock.unlock();
}

const std::string Service::getName() const
{
	return this->mName;
}

void Service::pushMsg(std::shared_ptr<ServiceMsg> msg)
{
	if (!this->mIsInit)
	{
		return;
	}

	while (!this->mMsgLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	this->mMsgs.push(msg);

	this->mMsgLock.unlock();
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
		auto serviceMsgPool = ServiceMsgPool::getInst();
		while (!this->mMsgLock.try_lock())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		lua_newtable(this->mState);
		auto index = 1;
		while(!this->mMsgs.empty())
		{
			auto it = this->mMsgs.front();
			this->mMsgs.pop();

			lua_newtable(this->mState);

			lua_pushinteger(this->mState, it->msgType);
			lua_setfield(this->mState, -2, "msgType");

			lua_pushinteger(this->mState, it->session);
			lua_setfield(this->mState, -2, "session");

			lua_pushinteger(this->mState, it->source);
			lua_setfield(this->mState, -2, "source");

			lua_pushstring(this->mState, reinterpret_cast<const char*>(it->data.data()));
			lua_setfield(this->mState, -2, "data");

			lua_pushstring(this->mState, it->error.c_str());
			lua_setfield(this->mState, -2, "error");

			lua_pushinteger(this->mState, it->status);
			lua_setfield(this->mState, -2, "status");

			lua_pushinteger(this->mState, it->fd);
			lua_setfield(this->mState, -2, "fd");

			lua_rawseti(this->mState, -2, index);
			index++;

			serviceMsgPool->push(it);
		}

		this->mMsgLock.unlock();

		if (lua_pcall(this->mState, 1, 0, 0) != LUA_OK)
		{
			spdlog::error("{}", lua_tostring(this->mState, -1));
		}
	}
}

