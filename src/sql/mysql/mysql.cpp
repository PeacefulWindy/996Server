#include<lua/luaApi.hpp>
#include<mysql/mysql.h>
#include <spdlog/spdlog.h>
#include <fmt/core.h>

int32_t newMariadb(lua_State* L)
{
	auto mysql = mysql_init(nullptr);
	if (!mysql)
	{
		lua_pushnil(L);
		return 1;
	}

	lua_pushlightuserdata(L, mysql);
	return 1;
}

int32_t destroyMariadb(lua_State* L)
{
	auto mysql = static_cast<MYSQL*>(lua_touserdata(L, 1));
	mysql_close(mysql);
	return 0;
}

int32_t connectMariadb(lua_State* L)
{
	auto mysql = static_cast<MYSQL*>(lua_touserdata(L, 1));
	auto host = luaL_checkstring(L, 2);
	auto port = luaL_checkinteger(L, 3);
	auto user = luaL_checkstring(L, 4);
	auto pwd = luaL_checkstring(L, 5);
	auto db = luaL_checkstring(L, 6);

	if (!mysql_real_connect(mysql, host, user, pwd, db, port, nullptr, 0))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, mysql_error(mysql));
		return 2;
	}

	lua_pushboolean(L, true);

	return 1;
}

int32_t prepareMariadb(lua_State* L)
{
	auto mysql = static_cast<MYSQL*>(lua_touserdata(L, 1));
	auto sql = luaL_checkstring(L, 2);

	auto stmt = mysql_stmt_init(mysql);
	if (mysql_stmt_prepare(stmt, sql, strlen(sql)))
	{
		lua_pushboolean(L, false);
		lua_pushstring(L, mysql_stmt_error(stmt));
		mysql_stmt_close(stmt);
		return 2;
	}

	lua_pushboolean(L, true);
	lua_pushlightuserdata(L, stmt);
	return 2;
}

int32_t queryMariadb(lua_State* L)
{
	auto newBadResult = [](lua_State* L, MYSQL* mysql)
		{
			lua_newtable(L);
			lua_pushboolean(L, true);
			lua_setfield(L, -2, "badresult");
			lua_pushinteger(L, mysql_errno(mysql));
			lua_setfield(L, -2, "errno");
			lua_pushstring(L, mysql_error(mysql));
			lua_setfield(L, -2, "error");
		};

	auto mysql = static_cast<MYSQL*>(lua_touserdata(L, 1));
	auto sql = luaL_checkstring(L, 2);
	if (mysql_query(mysql, sql))
	{
		newBadResult(L, mysql);
		return 1;
	}

	auto res = mysql_store_result(mysql);
	if (!res)
	{
		auto errId = mysql_errno(mysql);
		if (errId > 0)
		{
			newBadResult(L, mysql);
			return 1;
		}

		lua_newtable(L);
		lua_pushinteger(L, mysql_affected_rows(mysql));
		lua_setfield(L, -2, "affectedRows");

		auto insertId = mysql_insert_id(mysql);
		if (insertId > 0)
		{
			lua_pushinteger(L, insertId);
			lua_setfield(L, -2, "insertId");
		}

		return 1;
	}

	lua_newtable(L);

	auto fieldCount = mysql_num_fields(res);
	auto fields = mysql_fetch_fields(res);

	auto index = 1;
	auto row = mysql_fetch_row(res);

	while (row)
	{
		lua_newtable(L);
		for (auto i = 0; i < fieldCount; i++)
		{
			auto field = fields[i];
			switch (field.type)
			{
			case MYSQL_TYPE_STRING:
			case MYSQL_TYPE_VAR_STRING:
			case MYSQL_TYPE_BLOB:
				lua_pushstring(L, row[i]);
				break;
			case MYSQL_TYPE_TINY:
			case MYSQL_TYPE_SHORT:
			case MYSQL_TYPE_LONG:
			case MYSQL_TYPE_LONGLONG:
				lua_pushinteger(L, std::atoll(row[i]));
				break;
			case MYSQL_TYPE_DECIMAL:
			case MYSQL_TYPE_FLOAT:
			case MYSQL_TYPE_DOUBLE:
				lua_pushnumber(L, std::atof(row[i]));
				break;
			default:
				spdlog::error("mysql not support type:{}", static_cast<size_t>(field.type));
				continue;
			}

			lua_setfield(L, -2, field.name);
		}

		lua_rawseti(L, -2, index);
		index++;

		row=mysql_fetch_row(res);
	}

	mysql_free_result(res);

	return 1;
}

int32_t execMariadb(lua_State* L)
{
	auto newBadResult = [](lua_State* L,MYSQL_STMT* stmt)
		{
			lua_newtable(L);
			lua_pushboolean(L, true);
			lua_setfield(L, -2, "badresult");
			lua_pushinteger(L, mysql_stmt_errno(stmt));
			lua_setfield(L, -2, "errno");
			lua_pushstring(L, mysql_stmt_error(stmt));
			lua_setfield(L, -2, "error");
		};

	auto stmt = static_cast<MYSQL_STMT*>(lua_touserdata(L, 1));
	auto top = lua_gettop(L);

	auto argNum = mysql_stmt_param_count(stmt);
	if (argNum > top - 1)
	{
		lua_newtable(L);
		auto err = fmt::format("invalid args,need args num {},but get args num {}", argNum, top-1);
		lua_pushboolean(L, false);
		lua_setfield(L, -2, "badresult");
		lua_pushstring(L, err.c_str());
		lua_setfield(L, -2, "error");
		return 1;
	}

	auto params = std::vector<MYSQL_BIND>(argNum);
	auto valueParams = std::vector<std::vector<uint8_t>>(argNum);
	auto isNullParams = std::vector<my_bool>(argNum);

	for (auto i = 0; i < argNum; i++)
	{
		auto index = i + 2;
		auto type = lua_type(L, index);
		auto& param = params[i];
		auto& valueParam = valueParams[i];
		param.is_null = &isNullParams[i];

		auto value = std::string();
		switch (type)
		{
		case LUA_TSTRING:
			value = lua_tostring(L, index);
			break;
		case LUA_TNUMBER:
			if (lua_isinteger(L, index))
			{
				value = std::to_string(lua_tointeger(L, index));
			}
			else
			{
				value = std::to_string(lua_tonumber(L, index));
			}
			break;
		default:
			spdlog::error("not support lua type:{}", lua_typename(L, type));
			continue;
		}

		auto len = value.length();
		param.buffer_length = len;
		valueParam.resize(len + 1);
		memcpy(valueParam.data(), value.c_str(), len);
		param.buffer = valueParam.data();
	}

	if (mysql_stmt_bind_param(stmt, params.data()))
	{
		newBadResult(L, stmt);
		return 1;
	}

	if (mysql_stmt_execute(stmt))
	{
		newBadResult(L, stmt);
		return 1;
	}

	if (mysql_stmt_store_result(stmt))
	{
		newBadResult(L, stmt);
		return 1;
	}

	auto res = mysql_stmt_result_metadata(stmt);
	if (!res)
	{
		auto errId = mysql_stmt_errno(stmt);
		if (errId > 0)
		{
			newBadResult(L, stmt);
			return 1;
		}

		lua_newtable(L);
		lua_pushinteger(L, mysql_stmt_affected_rows(stmt));
		lua_setfield(L, -2, "affectedRows");

		auto insertId = mysql_stmt_insert_id(stmt);
		if (insertId > 0)
		{
			lua_pushinteger(L, insertId);
			lua_setfield(L, -2, "insertId");
		}
		return 1;
	}

	auto fieldCount = mysql_num_fields(res);
	auto fields = mysql_fetch_fields(res);

	auto results = std::vector<MYSQL_BIND>(fieldCount);
	auto dataResults = std::vector<std::vector<int8_t>>(fieldCount);
	auto isNullResults = std::vector<my_bool>(fieldCount);

	for (auto i = 0; i < fieldCount; i++)
	{
		auto field = fields[i];
		auto& it = results[i];
		auto& value = dataResults[i];
		auto len = field.length;
		value.resize(static_cast<size_t>(len) + 1);
		it.buffer_length = len;
		it.buffer = value.data();
		it.is_null = &isNullResults[i];
	}

	if (mysql_stmt_bind_result(stmt, results.data()))
	{
		newBadResult(L, stmt);
		mysql_stmt_free_result(stmt);
		return 1;
	}

	auto row = 1;
	auto ret = 0;
	lua_newtable(L);
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

		lua_newtable(L);
		for (auto i = 0; i < fieldCount; i++)
		{
			auto& result = results[i];
			auto& field = fields[i];
			if (*result.is_null)
			{
				continue;
			}

			auto value = (const char*)result.buffer;

			switch (field.type)
			{
			case MYSQL_TYPE_STRING:
			case MYSQL_TYPE_VAR_STRING:
			case MYSQL_TYPE_BLOB:
				lua_pushstring(L, value);
				break;
			case MYSQL_TYPE_TINY:
			case MYSQL_TYPE_SHORT:
			case MYSQL_TYPE_LONG:
			case MYSQL_TYPE_LONGLONG:
				lua_pushinteger(L, std::atoll(value));
				break;
			case MYSQL_TYPE_FLOAT:
			case MYSQL_TYPE_DOUBLE:
				lua_pushnumber(L, std::atof(value));
				break;
			default:
				spdlog::error("mysql not support type:{}", static_cast<size_t>(field.type));
				continue;
			}

			lua_setfield(L, -2, field.name);
		}

		lua_rawseti(L, -2, row);
		row++;
	}

	mysql_stmt_free_result(stmt);

	return 1;
}

int32_t stmtCloseMariadb(lua_State* L)
{
	auto stmt = static_cast<MYSQL_STMT*>(lua_touserdata(L, 1));
	mysql_stmt_close(stmt);
	return 0;
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

	lua_pushcfunction(state, prepareMariadb);
	lua_setfield(state, -2, "prepare");

	lua_pushcfunction(state, execMariadb);
	lua_setfield(state, -2, "exec");

	lua_pushcfunction(state, stmtCloseMariadb);
	lua_setfield(state, -2, "stmtClose");

	lua_pushcfunction(state, queryMariadb);
	lua_setfield(state, -2, "query");

	lua_setglobal(state, "mysql");
}