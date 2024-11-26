#include"httpServerMgr.hpp"
#include<http/server/httpSever.hpp>

HttpServerMgr* HttpServerMgr::mInst = nullptr;

HttpServerMgr* HttpServerMgr::getInst()
{
	if (!HttpServerMgr::mInst)
	{
		HttpServerMgr::mInst = new HttpServerMgr();
	}

	return HttpServerMgr::mInst;
}

int32_t HttpServerMgr::newServer()
{
	auto id = this->mAutoId;
	this->mAutoId++;

	auto httpServer = new HttpServer();

	while (!this->mLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	this->mServers.insert({ id,httpServer });
	this->mLock.unlock();

	return id;
}

HttpServer* HttpServerMgr::getServer(int32_t id)
{
	while (!this->mLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	auto iter = this->mServers.find(id);
	if (iter == this->mServers.end())
	{
		this->mLock.unlock();
		return nullptr;
	}

	auto ptr = iter->second;

	this->mLock.unlock();

	return ptr;
}

void HttpServerMgr::destroyServer(int32_t id)
{
	while (!this->mLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	auto iter = this->mServers.find(id);
	if (iter == this->mServers.end())
	{
		this->mLock.unlock();
		return;
	}
	this->mServers.erase(iter);

	this->mLock.unlock();
}
