#pragma once
#include<string>
#include<map>
#include<mutex>
#include<vector>

class Service;

class ServiceMgr
{
public:
	static ServiceMgr* getInst();
	static void destroy();

public:
	int32_t newService(std::string name, std::string src,bool isUnique);
	Service* getService(int32_t id);
	int32_t queryService(std::string name);
	void destoryService(int32_t id);
	
public:
	std::vector<std::string>& getServicePath();
	void addServicePath(std::string value);

private:
	virtual ~ServiceMgr();

private:
	static ServiceMgr* mInst;
	std::map<int32_t,Service*> mServices;
	int32_t mAutoId = 1;
	std::vector<std::string> mServicePaths;
	std::mutex mServiceLock;
};