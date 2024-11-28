#include<lua/luaApi.hpp>
#include<cryptopp/sha3.h>
#include<crypto/util/util.hpp>

template<class T>
std::string sha3(std::string input,uint32_t hashSize)
{
	auto hash = T();
	auto digest = std::vector<CryptoPP::byte>(hashSize);

	hash.Update(reinterpret_cast<const CryptoPP::byte*>(input.c_str()), input.length());
	hash.Final(digest.data());

	return dataToHex(reinterpret_cast<const char*>(digest.data()), digest.size());
}

int sha3_256(lua_State* L)
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

	auto hexDigest = sha3<CryptoPP::SHA3_256>(std::string(input, len), CryptoPP::SHA3_256::DIGESTSIZE);
	lua_pushstring(L, hexDigest.c_str());

	return 1;
}

int sha3_512(lua_State* L)
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

	auto hexDigest = sha3<CryptoPP::SHA3_512>(std::string(input, len), CryptoPP::SHA3_512::DIGESTSIZE);
	lua_pushstring(L, hexDigest.c_str());

	return 1;
}

void luaRegisterCryptoShaAPI(lua_State* state)
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

	lua_pushcfunction(state, sha3_256);
	lua_setfield(state, -2, "sha3_256");

	lua_pushcfunction(state, sha3_512);
	lua_setfield(state, -2, "sha3_512");

	if (!hasTable)
	{
		lua_setglobal(state, name);
	}
	else
	{
		lua_pop(state, 1);
	}
}