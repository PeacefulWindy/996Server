#pragma once
#include<string>
#include<lua/luaApi.hpp>

class Service
{
public:
	Service(int32_t id,std::string name, std::string src);
	virtual ~Service();

public:
	int32_t getId()const;
	const std::string getName()const;

public:
	void poll();

private:
	std::string mName;
	int32_t mId;
	lua_State* L;
	bool mIsInit = false;
};