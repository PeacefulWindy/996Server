#include<lua/luaApi.hpp>
#include<filesystem>
#include<spdlog/spdlog.h>

int32_t existsFs(lua_State* L)
{
	auto dirPath = luaL_checkstring(L, 1);
	auto path = std::filesystem::u8path(dirPath);

	if (!std::filesystem::exists(path))
	{
		lua_pushboolean(L,false);
		return 1;
	}

	lua_pushboolean(L, true);
	return 1;
}

int32_t listdir(lua_State* L)
{
	auto dirPath = luaL_checkstring(L, 1);
	auto path=std::filesystem::u8path(dirPath);

	lua_newtable(L);
	if (!std::filesystem::exists(path))
	{
		return 1;
	}

	auto index = 1;
	for (const auto& it : std::filesystem::directory_iterator(path))
	{
		if (std::filesystem::is_regular_file(it))
		{
			auto filePath = it.path().filename().string();
			lua_pushstring(L, filePath.c_str());
			lua_rawseti(L, -2, index);
		}
	}
	
	return 1;
}

int32_t mkdir(lua_State* L)
{
	auto dirPath = luaL_checkstring(L, 1);
	auto path = std::filesystem::u8path(dirPath);
	auto ec = std::error_code();
	if (!std::filesystem::create_directories(path, ec))
	{
		lua_pushboolean(L,false);

		if (ec)
		{
			lua_pushstring(L, ec.message().c_str());
		}
		return 1;
	}

	lua_pushboolean(L, true);
	return 1;
}

int32_t rmdir(lua_State* L)
{
	auto dirPath = luaL_checkstring(L, 1);
	auto path = std::filesystem::u8path(dirPath);
	auto ec = std::error_code();
	if (!std::filesystem::remove_all(path, ec))
	{
		lua_pushboolean(L, false);

		if (ec)
		{
			lua_pushstring(L, ec.message().c_str());
		}
		return 2;
	}

	lua_pushboolean(L, true);
	return 1;
}

void luaRegisterFsAPI(lua_State* state)
{
	lua_settop(state, 0);

	lua_newtable(state);

	lua_pushcfunction(state, listdir);
	lua_setfield(state, -2, "listdir");

	lua_pushcfunction(state, existsFs);
	lua_setfield(state, -2, "exists");

	lua_pushcfunction(state, mkdir);
	lua_setfield(state, -2, "mkdir");

	lua_pushcfunction(state, rmdir);
	lua_setfield(state, -2, "rmdir");

	lua_setglobal(state, "fs");
}