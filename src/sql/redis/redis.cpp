#include<lua/luaApi.hpp>
#include<hiredis/hiredis.h>
#include<spdlog/spdlog.h>

int32_t newRedis(lua_State* L)
{
	auto host = luaL_checkstring(L, 1);
	auto port = luaL_checkinteger(L, 2);

	auto context = redisConnect(host, port);
	if (!context)
	{
		spdlog::error("can't allocate redis context");
		return 0;
	}

	if (context->err)
	{
		spdlog::error(context->errstr);
		return 0;
	}

	lua_pushlightuserdata(L, context);
	return 1;
}

int32_t destroyRedis(lua_State* L)
{
	auto ptr = static_cast<redisContext*>(lua_touserdata(L, 1));
	redisFree(ptr);
	return 0;
}

void newTableFromRedisReply(lua_State * L,redisReply* reply,int32_t index,bool isRoot)
{
	switch (reply->type)
	{
	case REDIS_REPLY_ARRAY:
		if (!isRoot)
		{
			lua_newtable(L);
		}
		for (auto i = 0; i < reply->elements; i++)
		{
			newTableFromRedisReply(L, reply->element[i], i + 1, false);
		}
		if (!isRoot)
		{
			lua_rawseti(L, -2, index);
		}
		return;
	case REDIS_REPLY_INTEGER:
		lua_pushinteger(L, reply->integer);
		break;
	case REDIS_REPLY_NIL:
		lua_pushnil(L);
		break;
	case REDIS_REPLY_DOUBLE:
		lua_pushnumber(L, reply->dval);
		break;
	case REDIS_REPLY_STRING:
	case REDIS_REPLY_STATUS:
	case REDIS_REPLY_ERROR:
		lua_pushstring(L, reply->str);
		break;
	default:
		spdlog::error("Unknown redis type:{}", reply->type);
		return;
	}
	lua_rawseti(L, -2, index);
}

int32_t cmdRedis(lua_State* L)
{
	auto ptr= static_cast<redisContext*>(lua_touserdata(L, 1));
	auto cmd = luaL_checkstring(L, 2);
	auto reply = static_cast<redisReply*>(redisCommand(ptr, cmd));
	if (!reply)
	{
		spdlog::error("{}", ptr->errstr);
		return 0;
	}
	else
	{
		lua_newtable(L);
		newTableFromRedisReply(L, reply, 1, true);
	}

	freeReplyObject(reply);
	return 1;
}

void luaRegisterRedisAPI(lua_State* state)
{
	lua_settop(state, 0);

	lua_newtable(state);

	lua_pushcfunction(state, newRedis);
	lua_setfield(state, -2, "new");

	lua_pushcfunction(state, destroyRedis);
	lua_setfield(state, -2, "destroy");

	lua_pushcfunction(state, cmdRedis);
	lua_setfield(state, -2, "cmd");

	lua_setglobal(state, "redis");
}