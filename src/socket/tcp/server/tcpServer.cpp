#include "tcpServer.hpp"
#include <spdlog/spdlog.h>
#include<socket/tcp/session/tcpSession.hpp>

extern asio::io_context IoContext;

TcpServer::TcpServer(int32_t id)
	:mId(id)
{
	
}

TcpServer::~TcpServer()
{
	this->close();
}

bool TcpServer::listen(int32_t port, std::string host)
{
	if (this->mAcceptor)
	{
		return false;
	}

	auto endpoint = asio::ip::tcp::endpoint(asio::ip::make_address(host), port);
	this->mAcceptor = new asio::ip::tcp::acceptor(IoContext, endpoint);
	this->doAcceptor();
	return true;
}

void TcpServer::close()
{
	if (this->mAcceptor)
	{
		delete this->mAcceptor;
		this->mAcceptor = nullptr;

		while (!this->mLock.try_lock())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		this->mSessions.clear();

		this->mLock.unlock();
	}
}

void TcpServer::close(uint64_t fd)
{
	if (this->mOnCloseFunc)
	{
		this->mOnCloseFunc(fd);
	}

	while (!this->mLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	auto iter = this->mSessions.find(fd);
	if (iter == this->mSessions.end())
	{
		this->mLock.unlock();
		return;
	}

	this->mSessions.erase(iter);

	this->mLock.unlock();
}

void TcpServer::send(uint64_t fd, std::string data)
{
	while (!this->mLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	
	auto iter = this->mSessions.find(fd);
	if (iter == this->mSessions.end())
	{
		this->mLock.unlock();
		return;
	}

	auto session = iter->second;

	this->mLock.unlock();

	session->send(data);
}

void TcpServer::setOnConnectFunc(std::function<void(uint64_t)> func)
{
	this->mOnConnectFunc = func;
}

void TcpServer::setOnCloseFunc(std::function<void(uint64_t)> func)
{
	this->mOnCloseFunc = func;
}

void TcpServer::setOnMsgFunc(std::function<void(uint64_t, const std::string&)> func)
{
	this->mOnMsgFunc = func;
}

void TcpServer::onMsg(uint64_t fd, const std::string data)
{
	if (this->mOnMsgFunc)
	{
		this->mOnMsgFunc(fd, data);
	}
}

void TcpServer::onAcceptor(std::error_code ec, asio::ip::tcp::socket socket)
{
	if (ec)
	{
		spdlog::error("{}", ec.message());
		return;
	}

	while (!this->mLock.try_lock())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	auto fd = this->mAutoId;
	this->mAutoId++;

	auto session = std::make_shared<TcpSession>(this->mId, fd, std::move(socket));
	this->mSessions.insert({ fd,session });

	this->mLock.unlock();

	if (this->mOnConnectFunc)
	{
		this->mOnConnectFunc(fd);
	}

	this->doAcceptor();
}

void TcpServer::doAcceptor()
{
	this->mAcceptor->async_accept(std::bind(&TcpServer::onAcceptor, this, std::placeholders::_1, std::placeholders::_2));
}
