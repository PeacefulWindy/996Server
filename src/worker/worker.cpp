#include "worker.hpp"
#include<spdlog/spdlog.h>
#include<service/service.hpp>
#include <service/serviceMgr.hpp>

Worker::Worker(int32_t id)
	:mId(id)
{
	spdlog::info("worker_{} start", this->mId);
	this->mIsRun = true;
	this->mThread = std::make_unique<std::thread>(&Worker::run, this);
}

Worker::~Worker()
{
	this->mIsRun = false;
	if (this->mThread)
	{
		this->mThread->join();
	}

	spdlog::info("worker_{} stop", this->mId);
}

Service* Worker::getService()const
{
	return this->mService;
}

void Worker::setService(Service* value)
{
	this->mService = value;
}

void Worker::run()
{
	while (this->mIsRun)
	{
		auto service = this->mService;
		auto serviceMgr=ServiceMgr::getInst();
		auto serviceId = 0;
		if (!service)
		{
			serviceId = serviceMgr->getFreeServiceId();
			if (serviceId == 0)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				continue;
			}

			service = serviceMgr->getService(serviceId);

			if (!service)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				continue;
			}
		}

		service->poll();

		if (serviceId > 0)
		{
			serviceMgr->pushFreeServiceId(serviceId);
		}
	}
}
