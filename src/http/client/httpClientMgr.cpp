#include"httpClientMgr.hpp"

HttpClientMgr* HttpClientMgr::mInst = nullptr;

HttpClientMgr* HttpClientMgr::getInst()
{
	if (!HttpClientMgr::mInst)
	{
		HttpClientMgr::mInst = new HttpClientMgr();
	}

	return HttpClientMgr::mInst;
}

int32_t HttpClientMgr::newClient()
{
	while (!this->mLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	auto id = 0;
	if (this->mFreeClients.empty())
	{
		id = this->mAutoId;
		this->mAutoId++;
		auto client = new ix::HttpClient(true);
		this->mClients.insert({ id,client });
	}
	else
	{
		id = this->mFreeClients.front();
		this->mFreeClients.pop();
	}

	this->mLock.unlock();
	return id;
}

ix::HttpClient* HttpClientMgr::getClient(int32_t id)
{
	while (!this->mLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	auto iter = this->mClients.find(id);
	if (iter == this->mClients.end())
	{
		this->mLock.unlock();
		return nullptr;
	}

	auto client = iter->second;

	this->mLock.unlock();

	return client;
}

void HttpClientMgr::destroyClient(int32_t id)
{
	while (!this->mLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	auto iter = this->mClients.find(id);
	if (iter == this->mClients.end())
	{
		this->mLock.unlock();
		return;
	}

	this->mFreeClients.push(id);
	this->mLock.unlock();
}