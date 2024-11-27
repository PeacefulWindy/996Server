#pragma once

#pragma once
#include<asio.hpp>
#include<mutex>
#include<map>
#include<memory>
#include<define.hpp>

class TcpSession;

class TcpClient
{
public:
	TcpClient();
	virtual ~TcpClient();

public:
	bool connect(std::string host, int32_t port);
	void close();

public:
	void send(std::string data);

public:
	void setOnConnectFunc(std::function<void()> func);
	void setOnCloseFunc(std::function<void()> func);
	void setOnMsgFunc(std::function<void(const std::string)> func);

private:
	void onConnect(std::error_code ec, asio::ip::tcp::endpoint remoteInfo);
	void doRead();
	void onMsg(std::error_code ec, std::size_t length);

private:
	asio::ip::tcp::socket* mSocket = nullptr;
	std::function<void()> mOnConnectFunc;
	std::function<void()> mOnCloseFunc;
	std::function<void(const std::string)> mOnMsgFunc;
	std::array<char, TCP_PACK_SIZE> mData = { 0 };
};