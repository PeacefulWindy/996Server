#include<mysql/mysql.h>
#include<lua/luaApi.hpp>
#include <string.h>
#include <spdlog/spdlog.h>

int newMariadb(lua_State* state)
{
	auto mysql = mysql_init(nullptr);
	if (!mysql)
	{
		lua_pushnil(state);
		return 1;
	}

	lua_pushlightuserdata(state, mysql);

	return 1;
}

int destroyMariadb(lua_State* state)
{
	auto mysql = static_cast<MYSQL*>(lua_touserdata(state, 1));
	mysql_close(mysql);
	return 0;
}

int connectMariadb(lua_State* state)
{
	auto mysql = static_cast<MYSQL*>(lua_touserdata(state, 1));
	auto host = luaL_checkstring(state, 2);
	auto port = luaL_checkinteger(state, 3);
	auto user = luaL_checkstring(state, 4);
	auto pwd = luaL_checkstring(state, 5);
	auto db = luaL_checkstring(state, 6);

	if (!mysql_real_connect(mysql, host, user, pwd, db, port, nullptr, 0))
	{
		spdlog::error("{}", mysql_error(mysql));
		lua_pushboolean(state, false);
		return 1;
	}

	lua_pushboolean(state, true);

	return 1;
}

int newStmt(lua_State* state)
{
	auto mysql = static_cast<MYSQL*>(lua_touserdata(state, 1));
	auto stmt = mysql_stmt_init(mysql);

	if (!stmt)
	{
		lua_pushnil(state);
		return 1;
	}

	lua_pushlightuserdata(state, stmt);
	return 1;
}

int destroyStmt(lua_State* state)
{
	auto stmt = static_cast<MYSQL_STMT*>(lua_touserdata(state, 1));
	mysql_stmt_close(stmt);

	return 0;
}

int prepare(lua_State* state)
{
	auto stmt = static_cast<MYSQL_STMT*>(lua_touserdata(state, 1));
	auto sql = lua_tostring(state, 2);

	if (mysql_stmt_prepare(stmt, sql, strlen(sql)))
	{
		spdlog::error("mysql_stmt_prepare() failed: {}", mysql_stmt_error(stmt));

		lua_pushboolean(state, false);
		return 1;
	}

	lua_pushboolean(state, true);
	return 1;
}

int exec(lua_State* state)
{
	auto stmt = static_cast<MYSQL_STMT*>(lua_touserdata(state, 1));

	auto argBinds = std::vector<MYSQL_BIND>();
	auto freeBindFunc = [](std::vector<MYSQL_BIND>& binds)
		{
			for (auto it = binds.begin(); it != binds.end(); ++it)
			{
				free(it->buffer);
			}
		};

	if (lua_istable(state, 2))
	{
		lua_len(state, 2);
		auto argLen = lua_tointeger(state, -1);
		lua_pop(state, 1);
		if (argLen > 0)
		{
			argBinds.resize(argLen);

			lua_pushnil(state);
			const char* strData = nullptr;
			auto numberValue = 0.0f;
			auto intValue = 0;

			for (auto i = 0; i < argLen && lua_next(state, 2); i++)
			{
				auto type = lua_type(state, -1);
				auto& it = argBinds[i];

				switch (type)
				{
				case LUA_TSTRING:
					strData = lua_tostring(state, -1);
					it.buffer_type = MYSQL_TYPE_STRING;
					it.buffer_length = sizeof(char) * strlen(strData);
					it.buffer = malloc(it.buffer_length + 1);
					memcpy(it.buffer, strData, it.buffer_length);
					break;
				case LUA_TNUMBER:
					numberValue = lua_tonumber(state, -1);
					intValue = std::floor(numberValue);
					if (numberValue == intValue)
					{
						it.buffer_type = MYSQL_TYPE_LONG;
						it.buffer_length = sizeof(int32_t);
						it.buffer = malloc(it.buffer_length);
						memcpy(it.buffer, &intValue, it.buffer_length);
					}
					else
					{
						it.buffer_type = MYSQL_TYPE_FLOAT;
						it.buffer_length = sizeof(float);
						it.buffer = malloc(it.buffer_length);
						memcpy(it.buffer, &numberValue, it.buffer_length);
					}
					break;
				}

				lua_pop(state, 1);
			}

			if (mysql_stmt_bind_param(stmt, argBinds.data()))
			{
				spdlog::error("mysql_stmt_bind_param() failed!{}", mysql_stmt_error(stmt));
				freeBindFunc(argBinds);
				lua_pushnil(state);
				return 1;
			}
		}
	}

	if (mysql_stmt_execute(stmt))
	{
		spdlog::error("mysql_stmt_execute() failed: {}", mysql_stmt_error(stmt));
		freeBindFunc(argBinds);
		lua_pushnil(state);
		return 1;
	}

	auto metaRes = mysql_stmt_result_metadata(stmt);
	if (!metaRes)
	{
		freeBindFunc(argBinds);
		if (mysql_stmt_errno(stmt) != 0)
		{
			spdlog::error("mysql_stmt_result_metadata() failed!{}", mysql_stmt_error(stmt));
			lua_pushnil(state);
			return 1;
		}
		else
		{
			lua_newtable(state);
			return 1;
		}
	}

	auto fieldCount = mysql_num_fields(metaRes);
	auto fields = mysql_fetch_fields(metaRes);

	auto binds = std::vector<MYSQL_BIND>(fieldCount);
	auto isNull = std::vector<my_bool>(fieldCount);
	auto lengths = std::vector<unsigned long>(fieldCount);

	for (auto i = 0; i < fieldCount; i++)
	{
		auto& it = binds[i];
		it.buffer_type = fields[i].type;
		it.is_null = &isNull[i];
		it.length = &lengths[i];

		switch (fields[i].type)
		{
		case MYSQL_TYPE_STRING:
		case MYSQL_TYPE_VAR_STRING:
		case MYSQL_TYPE_BLOB:
			it.buffer_length = fields[i].length;
			it.buffer = malloc(sizeof(char) * fields[i].length);
			break;
		case MYSQL_TYPE_LONG:
			it.buffer_length = sizeof(int32_t);
			it.buffer = malloc(sizeof(int32_t));
			break;
		default:
			spdlog::error("Type not supported:{}", static_cast<size_t>(fields[i].type));
			break;
		}
	}

	if (mysql_stmt_bind_result(stmt, binds.data()))
	{
		spdlog::error("mysql_stmt_bind_result() failed!{}", mysql_stmt_error(stmt));
		mysql_stmt_free_result(stmt);
		freeBindFunc(argBinds);
		freeBindFunc(binds);
		lua_pushnil(state);
		return 1;
	}

	auto flag = 1;
	auto ret = 0;
	lua_newtable(state);
	while (true)
	{
		ret = mysql_stmt_fetch(stmt);
		if (ret == 1)
		{
			spdlog::error("mysql_stmt_fetch() failed!{}", mysql_stmt_error(stmt));
			break;
		}
		else if (ret == MYSQL_NO_DATA)
		{
			break;
		}

		lua_newtable(state);
		for (auto i = 0; i < fieldCount; i++)
		{
			if (*binds[i].is_null)
			{
				continue;
			}

			switch (binds[i].buffer_type)
			{
			case MYSQL_TYPE_STRING:
			case MYSQL_TYPE_VAR_STRING:
			case MYSQL_TYPE_BLOB:
				lua_pushstring(state, (const char*)binds[i].buffer);
				lua_setfield(state, -2, fields[i].name);
				break;
			case MYSQL_TYPE_LONG:
				lua_pushinteger(state, *(int*)binds[i].buffer);
				lua_setfield(state, -2, fields[i].name);
				break;
			default:
				break;
			}
		}

		lua_rawseti(state, -2, flag);
		flag++;
	}

	freeBindFunc(argBinds);
	freeBindFunc(binds);
	mysql_stmt_free_result(stmt);

	return 1;
}

int bindStmt(lua_State* state)
{
	auto size = lua_gettop(state) - 1;
	auto stmt = static_cast<MYSQL_STMT*>(lua_touserdata(state, 1));

	if (size <= 0)
	{
		lua_pushboolean(state, false);
		return 1;
	}

	auto args = std::vector<MYSQL_BIND>();
	for (auto i = 2; i < size + 2; i++)
	{
		auto type = lua_type(state, i);

		auto arg = MYSQL_BIND();
		if (type == LUA_TSTRING)
		{
			arg.buffer_type = MYSQL_TYPE_STRING;
			auto strPtr = lua_tostring(state, i);
			arg.buffer = &strPtr;
			arg.buffer_length = strlen(strPtr);
		}
		else if (type == LUA_TNUMBER)
		{
			arg.buffer_type = MYSQL_TYPE_STRING;
			auto floatPtr = lua_tonumber(state, i);
			arg.buffer = &floatPtr;
			arg.buffer_length = sizeof(float);
		}
		else
		{
			continue;
		}

		arg.is_null = 0;
		args.emplace_back(arg);
	}

	if (mysql_stmt_bind_param(stmt, args.data()))
	{
		spdlog::error("failed to bind params: %s", mysql_stmt_error(stmt));
		lua_pushboolean(state, false);
		return 1;
	}

	lua_pushboolean(state, true);
	return 1;
}

void luaRegisterMariadbAPI(lua_State* state)
{
	lua_settop(state, 0);

	lua_newtable(state);

	lua_pushcfunction(state, newMariadb);
	lua_setfield(state, -2, "new");

	lua_pushcfunction(state, destroyMariadb);
	lua_setfield(state, -2, "destroy");

	lua_pushcfunction(state, connectMariadb);
	lua_setfield(state, -2, "connect");

	lua_pushcfunction(state, newStmt);
	lua_setfield(state, -2, "new");

	lua_pushcfunction(state, prepare);
	lua_setfield(state, -2, "prepare");

	lua_pushcfunction(state, exec);
	lua_setfield(state, -2, "exec");

	lua_pushcfunction(state, destroyStmt);
	lua_setfield(state, -2, "destroyStmt");

	lua_pushcfunction(state, bindStmt);
	lua_setfield(state, -2, "bindStmt");

	lua_setglobal(state, "Mysql");
}