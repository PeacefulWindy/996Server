#include "websocketServerMgr.hpp"
#include<websocket/server/websocketServer.hpp>

WebsocketServerMgr* WebsocketServerMgr::mInst = nullptr;

WebsocketServerMgr* WebsocketServerMgr::getInst()
{
	if (!WebsocketServerMgr::mInst)
	{
		WebsocketServerMgr::mInst = new WebsocketServerMgr();
	}

	return WebsocketServerMgr::mInst;
}

int32_t WebsocketServerMgr::newServer()
{
	auto id = this->mAutoId;
	this->mAutoId++;

	auto weboscketServer = new WebsocketServer();

	while (!this->mLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	this->mServers.insert({ id,weboscketServer });
	this->mLock.unlock();

	return id;
}

WebsocketServer* WebsocketServerMgr::getServer(int32_t id)
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

void WebsocketServerMgr::destroyServer(int32_t id)
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
