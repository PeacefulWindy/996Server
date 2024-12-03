#pragma once
#include<asio.hpp>
#include <define.hpp>

class TcpSession : public std::enable_shared_from_this<TcpSession>
{
public:
	TcpSession(int32_t id, uint64_t fd, asio::ip::tcp::socket socket);

public:
	bool send(std::string& data);

public:
	std::shared_ptr<RemoteInfo> getRemoteInfo();

private:
	void doRead();
	void onMsg(std::error_code ec, std::size_t length);
	void close();

private:
	asio::ip::tcp::socket mSocket;
	int32_t mId;
	uint64_t mFd;
	std::array<char, TCP_PACK_SIZE> mData = { 0 };
};