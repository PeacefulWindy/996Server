#pragma once
#include<mutex>
#include<map>
#include<array>

class TcpClient;

class TcpClientMgr
{
public:
	static TcpClientMgr* getInst();

public:
	int32_t newClient();
	TcpClient* getClient(int32_t id);
	void destroyClient(int32_t id);

private:
	std::map<int32_t, TcpClient*> mClients;
	static TcpClientMgr* mInst;
	std::mutex mLock;
	int32_t mAutoId = 1;
};