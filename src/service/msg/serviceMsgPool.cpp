#include "serviceMsgPool.hpp"
#include<service/msg/serviceMsg.hpp>
#include<thread>

ServiceMsgPool* ServiceMsgPool::mInst = nullptr;

ServiceMsgPool* ServiceMsgPool::newInst(uint32_t num)
{
	if (!ServiceMsgPool::mInst)
	{
		ServiceMsgPool::mInst = new ServiceMsgPool(num);
	}

	return ServiceMsgPool::mInst;
}

ServiceMsgPool* ServiceMsgPool::getInst()
{
	return ServiceMsgPool::mInst;
}

ServiceMsgPool::ServiceMsgPool(uint32_t num)
{
	for (auto i = 0; i < num; i++)
	{
		this->mPool.push(std::make_shared<ServiceMsg>());
	}
}

void ServiceMsgPool::push(std::shared_ptr<ServiceMsg> value)
{
	while (!this->mLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	value->reset();
	this->mPool.push(value);

	this->mLock.unlock();
}

std::shared_ptr<ServiceMsg> ServiceMsgPool::pop()
{
	while (!this->mLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	auto ptr = this->mPool.front();
	this->mPool.pop();
	this->mLock.unlock();

	return ptr;
}

