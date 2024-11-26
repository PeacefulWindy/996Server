#pragma once
#include<map>
#include<mutex>

class HttpServer;

class HttpServerMgr
{
public:
	static HttpServerMgr* getInst();

public:
	int32_t newServer();
	HttpServer* getServer(int32_t id);
	void destroyServer(int32_t id);

private:
	static HttpServerMgr* mInst;
private:
	std::map<int32_t, HttpServer*> mServers;
	int32_t mAutoId = 1;
	std::mutex mLock;
};