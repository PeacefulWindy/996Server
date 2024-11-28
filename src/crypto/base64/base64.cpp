#include<lua/luaApi.hpp>
#include<cryptopp/base64.h>

int base64_encode(lua_State* L)
{
	auto top = lua_gettop(L);
	auto input = luaL_checkstring(L, 1);
	auto len = 0;
	if (top > 1)
	{
		len = luaL_checkinteger(L, 2);
	}
	else
	{
		len=strlen(input);
	}

	auto outputStr=std::string();
	CryptoPP::StringSource(input, len, new CryptoPP::Base64Encoder(new CryptoPP::StringSink(outputStr)));

	lua_pushlstring(L, outputStr.c_str(), outputStr.length());
	return 1;
}

int base64_decode(lua_State* L)
{
	auto top = lua_gettop(L);
	auto input = luaL_checkstring(L, 1);
	auto len = 0;
	if (top > 1)
	{
		len = luaL_checkinteger(L, 2);
	}
	else
	{
		len = strlen(input);
	}

	auto outputStr = std::string();
	CryptoPP::StringSource(input, len, new CryptoPP::Base64Decoder(new CryptoPP::StringSink(outputStr)));

	lua_pushlstring(L, outputStr.c_str(), outputStr.length());
	return 1;
}

void luaRegisterCryptoBase64API(lua_State* state)
{
	lua_settop(state, 0);

	auto name = "crypto";
	auto hasTable = true;
	lua_getglobal(state, name);
	if (lua_isnil(state, -1))
	{
		lua_newtable(state);
		hasTable = false;
	}

	lua_pushcfunction(state, base64_encode);
	lua_setfield(state, -2, "base64Encode");

	lua_pushcfunction(state, base64_decode);
	lua_setfield(state, -2, "base64Decode");

	if (!hasTable)
	{
		lua_setglobal(state, name);
	}
	else
	{
		lua_pop(state, 1);
	}
}