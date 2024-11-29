#pragma once
#include<vector>
#include<string>

enum class ServiceMsgType
{
	None = 0,
	Close = 1,
	Timer = 2,
	Lua = 3,
	LuaResponse = 4,
	HttpClient = 5,
	WebsocketServer = 6,
	//WebsocketClient = 7,
	TcpServer = 8,
	TcpClient = 9,
};

class ServiceMsg
{
public:
	uint32_t source;
	uint32_t session;
	uint32_t msgType;
	int32_t status;
	uint32_t fd;
	std::vector<uint8_t> data = std::vector<uint8_t>(4096);
	std::string error;

public:
	void reset();
};