#pragma once
#include<mutex>
#include<map>

class TcpServer;

class TcpServerMgr
{
public:
	static TcpServerMgr* getInst();

public:
	int32_t newServer();
	TcpServer* getServer(int32_t id);
	void destroyServer(int32_t id);
	void close(int32_t id, uint64_t fd);

private:
	std::map<int32_t, TcpServer*> mServers;
	static TcpServerMgr* mInst;
	std::mutex mLock;
	int32_t mAutoId = 1;
};