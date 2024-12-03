#include "serviceMsg.hpp"

void ServiceMsg::reset()
{
	this->source = 0;
	this->session = 0;
	this->msgType = 0;
	this->status = 0;
	this->dataLen = 0;
	this->isOk = false;
	memset(this->data.data(), 0, this->data.size());
	this->error.clear();
}
