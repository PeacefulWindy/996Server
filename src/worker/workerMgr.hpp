#pragma once
#include <cstdint>
#include <map>
#include<vector>
#include <memory>
#include<mutex>

class Service;
class Worker;

class WorkerMgr
{
public:
	static WorkerMgr* getInst();
	static void destroy();

public:
	void initWorker(int32_t num);

public:
	Worker* getWorker(size_t id);
	size_t useWorker();
	void freeWorker(size_t id);

public:
	Worker* getServiceWorker(Service* value);

private:
	virtual ~WorkerMgr();

private:
	static WorkerMgr* mInst;
	std::map<size_t, bool> mFreeWorkers;
	std::vector<Worker*> mWorkers;
	std::mutex mWorkerLock;
};