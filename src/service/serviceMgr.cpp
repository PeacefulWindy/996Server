#include "serviceMgr.hpp"
#include<worker/workerMgr.hpp>
#include<service/service.hpp>
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

int32_t ServiceMgr::newService(std::string name,std::string src,bool isUnique)
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

	auto service = new Service(id,name, src);

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

	auto iter=this->mServices.find(id);
	if (iter == this->mServices.end())
	{
		this->mServiceLock.unlock();
		return;
	}

	auto workerMgr = WorkerMgr::getInst();
	auto worker = workerMgr->getServiceWorker(iter->second);
	if (worker)
	{
		worker->setService(nullptr);
	}

	delete iter->second;
	this->mServices.erase(iter);

	this->mServiceLock.unlock();
}

std::vector<std::string>& ServiceMgr::getServicePath()
{
	return this->mServicePaths;
}

void ServiceMgr::addServicePath(std::string value)
{
	this->mServicePaths.emplace_back(value);
}

ServiceMgr::~ServiceMgr()
{
	while (!this->mServiceLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	auto workerMgr = WorkerMgr::getInst();
	for (auto it = this->mServices.begin(); it != this->mServices.end(); ++it)
	{
		auto worker = workerMgr->getServiceWorker(it->second);
		if (worker)
		{
			worker->setService(nullptr);
		}

		delete it->second;
	}

	this->mServices.clear();

	this->mServiceLock.unlock();
}
