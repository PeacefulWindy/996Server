#include"websocketServer.hpp"
#include<spdlog/spdlog.h>

WebsocketServer::WebsocketServer()
{

}

WebsocketServer::~WebsocketServer()
{
	this->close();
}

bool WebsocketServer::listen(uint16_t port, std::string host, int32_t maxConnection)
{
	if (this->mServer)
	{
		return false;
	}

	this->mServer = new ix::WebSocketServer(port, host, ix::SocketServer::kDefaultTcpBacklog, maxConnection);
	this->mServer->setOnClientMessageCallback(std::bind(&WebsocketServer::onServerMsg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	auto ret=this->mServer->listen();
	if (!ret.first)
	{
		spdlog::error("{}", ret.second);
		return false;
	}
	this->mServer->start();

	return true;
}

void WebsocketServer::send(uint64_t fd, std::string data)
{
	while (!this->mLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	auto iter=this->mSessions.find(fd);
	if (iter == this->mSessions.end())
	{
		this->mLock.unlock();
		return;
	}

	iter->second->send(data, true);

	this->mLock.unlock();
}

void WebsocketServer::close()
{
	if (this->mServer)
	{
		this->mOnConnetionFunc = nullptr;
		this->mOnMsgFunc = nullptr;

		this->mServer->stop();
		delete this->mServer;
		this->mServer = nullptr;

		this->mReverseSessions.clear();
		this->mSessions.clear();

		this->mAutoId = 1;
	}
}

void WebsocketServer::close(uint64_t fd)
{
	while (!this->mLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	auto iter=this->mSessions.find(fd);
	if (iter == this->mSessions.end())
	{
		this->mLock.unlock();
		return;
	}
	auto socket = iter->second;
	this->mLock.unlock();

	socket->close();
}

void WebsocketServer::setOnConnectFunc(std::function<void(uint64_t)> func)
{
	this->mOnConnetionFunc = func;
}

void WebsocketServer::setOnMsgFunc(std::function<void(uint64_t, const std::string& msg)> func)
{
	this->mOnMsgFunc = func;
}

void WebsocketServer::setOnCloseFunc(std::function<void(uint64_t)> func)
{
	this->mOnCloseFunc = func;
}

void WebsocketServer::onServerMsg(std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket& webSocket, const ix::WebSocketMessagePtr& msg)
{
	switch (msg->type)
	{
	case ix::WebSocketMessageType::Open:
	{
		auto fd = this->mAutoId;
		this->mAutoId++;
		auto ptr = &webSocket;

		while (!this->mLock.try_lock())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		this->mSessions.insert({ fd,ptr });
		this->mReverseSessions.insert({ ptr,fd });

		this->mLock.unlock();

		if (this->mOnConnetionFunc)
		{
			this->mOnConnetionFunc(fd);
		}
		break;
	}
	case ix::WebSocketMessageType::Close:
	{
		while (!this->mLock.try_lock())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		auto iter = this->mReverseSessions.find(&webSocket);
		if (iter == this->mReverseSessions.end())
		{
			this->mLock.unlock();
			return;
		}

		auto iter2 = this->mSessions.find(iter->second);
		if (iter2 != this->mSessions.end())
		{
			this->mSessions.erase(iter2);
		}

		this->mReverseSessions.erase(iter);

		this->mLock.unlock();

		break;
	}
	case ix::WebSocketMessageType::Message:
	{
		while (!this->mLock.try_lock())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		auto iter = this->mReverseSessions.find(&webSocket);
		if (iter == this->mReverseSessions.end())
		{
			this->mLock.unlock();
			return;
		}

		auto id = iter->second;
		this->mLock.unlock();

		if (this->mOnMsgFunc)
		{
			this->mOnMsgFunc(id, msg->str);
		}

		break;
	}
	}
}
