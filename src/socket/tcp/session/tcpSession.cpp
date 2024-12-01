#include "tcpSession.hpp"
#include<spdlog/spdlog.h>
#include<socket/tcp/server/tcpServerMgr.hpp>
#include<socket/tcp/server/tcpServer.hpp>

TcpSession::TcpSession(int32_t id, uint64_t fd, asio::ip::tcp::socket socket)
	:mId(id), mFd(fd), mSocket(std::move(socket))
{
	this->doRead();
}

void TcpSession::send(std::string &data)
{
	auto ec = std::error_code();
	this->mSocket.write_some(asio::buffer(data), ec);
	//if (ec)
	//{
	//	this->close();
	//}
}

void TcpSession::doRead()
{
	this->mSocket.async_read_some(asio::buffer(this->mData), std::bind(&TcpSession::onMsg, this, std::placeholders::_1, std::placeholders::_2));
}

void TcpSession::onMsg(std::error_code ec, std::size_t length)
{
	if (ec)
	{
		/*this->close();*/
		return;
	}

	if (length == 0)
	{
		this->doRead();
		return;
	}

	auto tcpServerMgr = TcpServerMgr::getInst();
	auto tcpServer = tcpServerMgr->getServer(this->mId);
	if (!tcpServer)
	{
		return;
	}

	tcpServer->onMsg(this->mFd, std::string(reinterpret_cast<const char*>(this->mData.data()), length));

	this->doRead();
}

void TcpSession::close()
{
	auto tcpServerMgr = TcpServerMgr::getInst();
	tcpServerMgr->close(this->mId, this->mFd);
}
