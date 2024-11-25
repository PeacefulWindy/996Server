#pragma once
#include<thread>

class Service;

class Worker
{
public:
	Worker(int32_t id);
	virtual ~Worker();

public:
	Service* getService()const;
	void setService(Service* value);

public:
	void run();

private:
	std::unique_ptr<std::thread> mThread;
	int32_t mId;
	bool mIsRun = false;
	Service* mService = nullptr;
};