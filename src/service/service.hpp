#pragma once
#include<string>
#include<lua/luaApi.hpp>
#include<queue>
#include<vector>
#include<memory>
#include<mutex>
#include<service/msg/serviceMsg.hpp>

class Service
{
public:
	Service(int32_t id, std::string name, std::string src, std::string args);
	virtual ~Service();

public:
	void close();

public:
	int32_t getId()const;
	const std::string getName()const;

public:
	void pushMsg(std::shared_ptr<ServiceMsg> msg);

public:
	void poll();

private:
	std::string mName;
	int32_t mId;
	lua_State* mState = nullptr;
	bool mIsInit = false;
	std::queue<std::shared_ptr<ServiceMsg>> mMsgs;
	std::mutex mMsgLock;
};