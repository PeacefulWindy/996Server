#pragma once
#include<ixwebsocket/IXWebSocketServer.h>

constexpr const char* WebsocketDefaultHost = "127.0.0.1";
constexpr uint32_t WebsocketMaxConnection = 1000;

enum class WebsocketMsgType
{
	Open=1,
	Close=2,
	Msg=3,
};

class WebsocketServer
{
public:
	WebsocketServer();
	virtual ~WebsocketServer();

public:
	bool listen(uint16_t port, std::string host= WebsocketDefaultHost, int32_t maxConnection = WebsocketMaxConnection);

public:
	void send(uint32_t fd, std::string data);

public:
	void close();

public:
	void setOnConnectFunc(std::function<void(uint32_t)> func);
	void setOnCloseFunc(std::function<void(uint32_t)> func);
	void setOnMsgFunc(std::function<void(uint32_t, const std::string& msg)> func);

private:
	void onServerMsg(std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket& webSocket, const ix::WebSocketMessagePtr& msg);

private:
	ix::WebSocketServer* mServer = nullptr;
	uint32_t mAutoId = 1;
	std::map<uint32_t, ix::WebSocket*> mSessions;
	std::map<ix::WebSocket*, uint32_t> mReverseSessions;

	std::function<void(uint32_t)> mOnConnetionFunc;
	std::function<void(uint32_t)> mOnCloseFunc;
	std::function<void(uint32_t, const std::string& msg)> mOnMsgFunc;
	std::mutex mLock;
};