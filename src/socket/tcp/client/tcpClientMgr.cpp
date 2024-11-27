#include "tcpClientMgr.hpp"
#include<socket/tcp/client/tcpClient.hpp>

TcpClientMgr* TcpClientMgr::mInst = nullptr;

TcpClientMgr* TcpClientMgr::getInst()
{
	if (!TcpClientMgr::mInst)
	{
		TcpClientMgr::mInst = new TcpClientMgr();
	}

	return TcpClientMgr::mInst;
}

int32_t TcpClientMgr::newClient()
{
	auto id = this->mAutoId;
	this->mAutoId++;

	auto tcpClient = new TcpClient();

	while (!this->mLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	this->mClients.insert({ id,tcpClient });
	this->mLock.unlock();

	return id;
}

TcpClient* TcpClientMgr::getClient(int32_t id)
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

	auto ptr = iter->second;

	this->mLock.unlock();

	return ptr;
}

void TcpClientMgr::destroyClient(int32_t id)
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
	this->mClients.erase(iter);

	this->mLock.unlock();
}
