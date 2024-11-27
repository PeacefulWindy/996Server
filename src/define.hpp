#pragma once

constexpr size_t TCP_PACK_SIZE = 4096;

enum class TcpMsgType
{
	Open = 1,
	Close = 2,
	Msg = 3,
};