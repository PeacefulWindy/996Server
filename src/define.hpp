#pragma once

constexpr size_t TCP_PACK_SIZE = 4096;

enum class TcpMsgType
{
	Connect = 1,
	ConnectError = 2,
	Close = 3,
	Msg = 4,
};

struct RemoteInfo
{
	std::string host;
	uint16_t port;
};