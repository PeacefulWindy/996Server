#include "serviceMgr.hpp"
#include<worker/workerMgr.hpp>
#include<spdlog/spdlog.h>
#include<worker/worker.hpp>

ServiceMgr* ServiceMgr::mInst = nullptr;

ServiceMgr* ServiceMgr::getInst()
{
	if (!ServiceMgr::mInst)
	{
		ServiceMgr::mInst = new ServiceMgr();
	}

	return ServiceMgr::mInst;
}

void ServiceMgr::destroy()
{
	if(ServiceMgr::mInst)
	{
		delete ServiceMgr::mInst;
		ServiceMgr::mInst = nullptr;
	}
}

int32_t ServiceMgr::newService(std::string name,std::string src, std::string args,bool isUnique)
{
	auto workerMgr = WorkerMgr::getInst();
	auto workerId = 0;
	while (workerId == 0)
	{
		workerId = workerMgr->useWorker();
		if (workerId != 0)
		{
			break;
		}

		spdlog::warn("no worker,retry!");
	}

	auto worker = workerMgr->getWorker(workerId);

	auto id = this->mAutoId;
	this->mAutoId++;

	auto service = new Service(id, name, src, args);

	while (!this->mServiceLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	this->mServices.insert({ id,service });

	this->mServiceLock.unlock();

	if (isUnique)
	{
		worker->setService(service);
	}
	else
	{
		this->mFreeServices.push(id);
	}

	return id;
}

Service* ServiceMgr::getService(int32_t id)
{
	while (!this->mServiceLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	auto iter = this->mServices.find(id);
	if (iter == this->mServices.end())
	{
		this->mServiceLock.unlock();
		return nullptr;
	}

	auto service = iter->second;

	this->mServiceLock.unlock();

	return service;
}

int32_t ServiceMgr::queryService(std::string name)
{
	while (!this->mServiceLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	auto id = 0;
	for (auto it = this->mServices.begin(); it != this->mServices.end(); ++it)
	{
		if (it->second->getName() == name)
		{
			id = it->first;
			break;
		}
	}

	this->mServiceLock.unlock();
	return id;
}

void ServiceMgr::destoryService(int32_t id)
{
	while (!this->mServiceLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	auto iter = this->mServices.find(id);
	if (iter == this->mServices.end())
	{
		this->mServiceLock.unlock();
		return;
	}

	auto service = iter->second;
	auto workerMgr = WorkerMgr::getInst();
	auto worker = workerMgr->getServiceWorker(service);
	if (worker)
	{
		worker->setService(nullptr);
	}

	this->mServices.erase(iter);

	while (!this->mPreDestroyServiceLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	this->mPreDestroyServices.push(service);

	this->mServiceLock.unlock();
	this->mPreDestroyServiceLock.unlock();
}

void ServiceMgr::closeAllService()
{
	while (!this->mServiceLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	auto ids = std::vector<int32_t>();
	for (auto it = this->mServices.begin(); it != this->mServices.end(); ++it)
	{
		ids.emplace_back(it->first);
	}

	this->mServiceLock.unlock();

	for (auto it = ids.begin(); it != ids.end(); ++it)
	{
		this->destoryService(*it);
	}
}

void ServiceMgr::poll()
{
	while (!this->mPreDestroyServiceLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	while (!this->mPreDestroyServices.empty())
	{
		auto service = this->mPreDestroyServices.top();
		delete service;
		this->mPreDestroyServices.pop();
	}

	this->mPreDestroyServiceLock.unlock();
}

std::vector<std::string>& ServiceMgr::getServicePath()
{
	return this->mServicePaths;
}

void ServiceMgr::addServicePath(std::string value)
{
	this->mServicePaths.emplace_back(value);
}

void ServiceMgr::send(int32_t serviceId, std::shared_ptr<ServiceMsg> msg)
{
	auto service = this->getService(serviceId);
	if (!service)
	{
		return;
	}

	service->pushMsg(msg);
}

int32_t ServiceMgr::getFreeServiceId()
{
	while (!this->mFreeServiceLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	if (this->mFreeServices.size() == 0)
	{
		this->mFreeServiceLock.unlock();
		return 0;
	}

	auto id = this->mFreeServices.front();
	this->mFreeServices.pop();

	this->mFreeServiceLock.unlock();

	return id;
}

void ServiceMgr::pushFreeServiceId(int32_t value)
{
	while (!this->mFreeServiceLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	this->mFreeServices.push(value);

	this->mFreeServiceLock.unlock();
}

ServiceMgr::~ServiceMgr()
{
	this->poll();
}
