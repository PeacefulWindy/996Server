#pragma once
#include<mutex>
#include<map>

class WebsocketServer;

class WebsocketServerMgr
{
public:
	static WebsocketServerMgr* getInst();

public:
	int32_t newServer();
	WebsocketServer* getServer(int32_t id);
	void destroyServer(int32_t id);

private:
	std::map<int32_t, WebsocketServer*> mServers;
	static WebsocketServerMgr* mInst;
	std::mutex mLock;
	int32_t mAutoId = 1;
};