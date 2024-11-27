#pragma once
#include<asio.hpp>

constexpr size_t TCP_PACK_SIZE = 4096;

enum class TcpSessionType
{
	Server=1,
	Client=2,
};

class TcpSession : public std::enable_shared_from_this<TcpSession>
{
public:
	TcpSession(int32_t id, uint64_t fd, TcpSessionType type, asio::ip::tcp::socket socket);

public:
	void send(std::string& data);

private:
	void doRead();
	void onMsg(std::error_code ec, std::size_t length);
	void close();

private:
	asio::ip::tcp::socket mSocket;
	int32_t mId;
	uint64_t mFd;
	TcpSessionType mType;
	std::array<char, TCP_PACK_SIZE> mData;
};