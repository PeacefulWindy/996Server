#pragma once
#include<queue>
#include<memory>
#include<mutex>
#include<ixwebsocket/IXHttpClient.h>

struct HttpClientResponse
{
	int32_t status = 0;
	std::string body;
	std::string error;
};

class HttpClientMgr
{
public:
	static HttpClientMgr* getInst();

public:
	int32_t newClient();
	ix::HttpClient* getClient(int32_t id);
	void destroyClient(int32_t id);

private:
	std::queue<int32_t> mFreeClients;
	std::map<int32_t,ix::HttpClient*> mClients;
	static HttpClientMgr* mInst;
	std::mutex mLock;
	int32_t mAutoId = 1;
};