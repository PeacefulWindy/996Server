#include "tcpClient.hpp"
#include <spdlog/spdlog.h>
#include<socket/tcp/session/tcpSession.hpp>

extern asio::io_context IoContext;

TcpClient::TcpClient()
{
	this->mSocket = new asio::ip::tcp::socket(IoContext);
}

TcpClient::~TcpClient()
{
	this->close();

	if (this->mSocket)
	{
		delete this->mSocket;
		this->mSocket = nullptr;
	}
}

bool TcpClient::connect(std::string host,int32_t port)
{
	if (this->mSocket->is_open())
	{
		return false;
	}

	auto resolver = asio::ip::tcp::resolver(IoContext);
	auto endPoint= asio::ip::tcp::endpoint(asio::ip::make_address(host), port);
	auto ec = std::error_code();
	auto resolve = resolver.resolve(endPoint, ec);
	if (ec)
	{
		spdlog::error("{}", ec.message());
		return false;
	}

	asio::async_connect(*this->mSocket, resolve, std::bind(&TcpClient::onConnect, this, std::placeholders::_1, std::placeholders::_2));

	return true;
}

void TcpClient::close()
{
	if (!this->mSocket->is_open())
	{
		return;
	}

	this->mSocket->close();
}

void TcpClient::send(std::string data)
{
	if (!this->mSocket->is_open())
	{
		return;
	}

	this->mSocket->write_some(asio::buffer(data));
}

void TcpClient::setOnConnectFunc(std::function<void()> func)
{
	this->mOnConnectFunc = func;
}

void TcpClient::setOnCloseFunc(std::function<void()> func)
{
	this->mOnCloseFunc = func;
}

void TcpClient::setOnMsgFunc(std::function<void(const std::string)> func)
{
	this->mOnMsgFunc = func;
}

void TcpClient::onMsg(std::error_code ec, std::size_t length)
{
	if (ec)
	{
		this->close();
		return;
	}

	if (this->mOnMsgFunc)
	{
		this->mOnMsgFunc(std::string(this->mData.data(), length));
	}
}

void TcpClient::onConnect(std::error_code ec, asio::ip::tcp::endpoint endpoint)
{
	if (ec)
	{
		spdlog::error("{}", ec.message());
		return;
	}

	if (this->mOnConnectFunc)
	{
		this->mOnConnectFunc();
	}

	this->doRead();
}

void TcpClient::doRead()
{
	this->mSocket->async_read_some(asio::buffer(this->mData), std::bind(&TcpClient::onMsg, this, std::placeholders::_1, std::placeholders::_2));
}
