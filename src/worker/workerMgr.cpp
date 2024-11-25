#include"workerMgr.hpp"
#include"worker.hpp"

WorkerMgr* WorkerMgr::mInst = nullptr;

WorkerMgr* WorkerMgr::getInst()
{
	if (!WorkerMgr::mInst)
	{
		WorkerMgr::mInst = new WorkerMgr();
	}

	return WorkerMgr::mInst;
}

void WorkerMgr::destroy()
{
	if (WorkerMgr::mInst)
	{
		delete WorkerMgr::mInst;
		WorkerMgr::mInst = nullptr;
	}
}

void WorkerMgr::initWorker(int32_t num)
{
	for (auto i = 1; i <= num; i++)
	{
		auto worker=new Worker(i);
		this->mWorkers.emplace_back(worker);
		this->mFreeWorkers.insert({ i,true });
	}
}

Worker* WorkerMgr::getWorker(size_t id)
{
	if (id<1 || id > this->mWorkers.size())
	{
		return nullptr;
	}

	return this->mWorkers[id - 1];
}

size_t WorkerMgr::useWorker()
{
	while (!this->mWorkerLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	auto iter = this->mFreeWorkers.begin();
	auto id = iter->first;
	this->mFreeWorkers.erase(iter);
	this->mWorkerLock.unlock();
	return id;
}

void WorkerMgr::freeWorker(size_t id)
{
	if (id<1 || id > this->mWorkers.size())
	{
		return;
	}

	while (!this->mWorkerLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	this->mFreeWorkers.insert({ id,true });

	this->mWorkerLock.unlock();
}

Worker* WorkerMgr::getServiceWorker(Service* value)
{
	while (!this->mWorkerLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	Worker* worker = nullptr;

	for (auto it = this->mWorkers.begin(); it != this->mWorkers.end(); ++it)
	{
		if ((*it)->getService() == value)
		{
			worker = *it;
			break;
		}
	}

	this->mWorkerLock.unlock();
	return worker;
}

WorkerMgr::~WorkerMgr()
{
	for (auto it = this->mWorkers.begin(); it != this->mWorkers.end(); ++it)
	{
		delete* it;
	}
	this->mWorkers.clear();
}
