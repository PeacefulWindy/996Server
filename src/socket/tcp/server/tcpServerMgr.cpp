#include "tcpServerMgr.hpp"
#include<socket/tcp/server/tcpServer.hpp>

TcpServerMgr* TcpServerMgr::mInst = nullptr;

TcpServerMgr* TcpServerMgr::getInst()
{
	if (!TcpServerMgr::mInst)
	{
		TcpServerMgr::mInst = new TcpServerMgr();
	}

	return TcpServerMgr::mInst;
}

int32_t TcpServerMgr::newServer()
{
	auto id = this->mAutoId;
	this->mAutoId++;

	auto tcpServer = new TcpServer(id);

	while (!this->mLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	this->mServers.insert({ id,tcpServer });
	this->mLock.unlock();

	return id;
}

TcpServer* TcpServerMgr::getServer(int32_t id)
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

void TcpServerMgr::destroyServer(int32_t id)
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

void TcpServerMgr::close(int32_t id, uint64_t fd)
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

	auto server = iter->second;
	server->close(fd);

	this->mLock.unlock();
}
