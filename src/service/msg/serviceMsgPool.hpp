#pragma once
#include<queue>
#include<memory>
#include<mutex>

class ServiceMsg;

class ServiceMsgPool
{
public:
	static ServiceMsgPool* newInst(uint32_t num = 0);
	static ServiceMsgPool* getInst();

public:
	ServiceMsgPool(uint32_t num);

public:
	void push(std::shared_ptr<ServiceMsg> value);
	std::shared_ptr<ServiceMsg> pop();

private:
	std::queue<std::shared_ptr<ServiceMsg>> mPool;
	std::mutex mLock;
	static ServiceMsgPool* mInst;
};