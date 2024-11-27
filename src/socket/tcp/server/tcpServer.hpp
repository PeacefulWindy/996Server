#pragma once
#include<asio.hpp>
#include<mutex>
#include<map>
#include<memory>

constexpr const char* TcpServerDefaultHost = "127.0.0.1";

class TcpSession;

class TcpServer
{
public:
	TcpServer(int32_t id);
	virtual ~TcpServer();

public:
	bool listen(int32_t port, std::string host= TcpServerDefaultHost);
	void close();
	void close(uint64_t fd);

public:
	void send(uint64_t fd, std::string data);

public:
	void setOnConnectFunc(std::function<void(uint64_t)> func);
	void setOnCloseFunc(std::function<void(uint64_t)> func);
	void setOnMsgFunc(std::function<void(uint64_t, const std::string&)> func);

public:
	void onMsg(uint64_t fd, const std::string data);

private:
	void onAcceptor(std::error_code ec, asio::ip::tcp::socket socket);
	void doAcceptor();

private:
	int32_t mId;
	asio::ip::tcp::acceptor* mAcceptor = nullptr;
	uint64_t mAutoId = 1;
	std::map<uint64_t, std::shared_ptr<TcpSession>> mSessions;
	std::mutex mLock;

	std::function<void(uint64_t)> mOnConnectFunc;
	std::function<void(uint64_t)> mOnCloseFunc;
	std::function<void(uint64_t, const std::string&)> mOnMsgFunc;
};