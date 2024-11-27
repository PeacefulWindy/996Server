#pragma once
#include<vector>
#include<string>

enum class ServiceMsgType
{
	None = 0,
	Close=1,
	Lua = 2,
	LuaResponse = 3,
	HttpClient = 4,
	WebsocketServer = 5,
	//WebsocketClient = 6,
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