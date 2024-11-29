#include<lua/luaApi.hpp>
#include<cjson/cJSON.h>
#include <string>

void fetchCreateLuaTable(lua_State* state, cJSON* node)
{
	auto size = cJSON_GetArraySize(node);
	if (size <= 0)
	{
		return;
	}

	auto index = 1;
	for (auto i = 0; i < size; i++)
	{
		auto childNode = cJSON_GetArrayItem(node, i);
		if (!childNode)
		{
			continue;
		}

		switch (childNode->type)
		{
		case cJSON_Array:
		case cJSON_Object:
			lua_newtable(state);
			fetchCreateLuaTable(state, childNode);
			break;
		case cJSON_String:
			lua_pushstring(state, childNode->valuestring);
			break;
		case cJSON_Number:
			if (childNode->valueint == childNode->valuedouble)
			{
				lua_pushinteger(state, childNode->valueint);
			}
			else
			{
				lua_pushnumber(state, childNode->valuedouble);
			}
			break;
		case cJSON_True:
			lua_pushboolean(state, true);
			break;
		case cJSON_False:
			lua_pushboolean(state, false);
			break;
		}

		if (childNode->string)
		{
			lua_setfield(state, -2, childNode->string);
		}
		else
		{
			lua_rawseti(state, -2, index);
			index++;
		}
	}
}

cJSON* fetchCreateJson(lua_State* state)
{
	cJSON* root = nullptr;

	auto index = lua_gettop(state);
	lua_pushnil(state);

	auto isArray = false;

	while (lua_next(state, index) != 0)
	{
		cJSON* item = nullptr;

		if (!root)
		{
			if (lua_type(state, -2) != LUA_TSTRING)
			{
				root = cJSON_CreateArray();
				isArray = true;
			}
			else
			{
				root = cJSON_CreateObject();
			}
		}

		const char* key = nullptr;
		if (!isArray)
		{
			key = lua_tostring(state, -2);
		}

		auto typeId = lua_type(state, -1);
		switch (typeId)
		{
		case LUA_TBOOLEAN:
			item = cJSON_CreateBool(lua_toboolean(state, -1));
			break;
		case LUA_TNUMBER:
			if (lua_isinteger(state, -1))
			{
				item = cJSON_CreateNumber(lua_tointeger(state, -1));
			}
			else
			{
				item = cJSON_CreateNumber(lua_tonumber(state, -1));
			}
			break;
		case LUA_TSTRING:
			item = cJSON_CreateString(lua_tostring(state, -1));
			break;
		case LUA_TTABLE:
			item = fetchCreateJson(state);
			break;
		default:
			break;
		}

		if (!root)
		{
			if (isArray)
			{
				root = cJSON_CreateArray();
			}
			else
			{
				root = cJSON_CreateObject();
			}
		}

		if (item)
		{
			if (isArray)
			{
				cJSON_AddItemToArray(root, item);
			}
			else
			{
				cJSON_AddItemToObject(root, key, item);
			}
		}

		lua_pop(state, 1);
	}

	return root;
}

int decode(lua_State* state)
{
	auto jsonStr = lua_tostring(state, 1);
	lua_settop(state, 0);

	auto root = cJSON_Parse(jsonStr);
	if (!root)
	{
		lua_pushnil(state);
		return 1;
	}

	lua_newtable(state);
	fetchCreateLuaTable(state, root);
	cJSON_Delete(root);

	return 1;
}

int encode(lua_State* state)
{
	if (!lua_istable(state, 1))
	{
		lua_pushstring(state, "{}");
		return 1;
	}

	auto root = fetchCreateJson(state);
	if (!root)
	{
		lua_pushstring(state, "{}");
	}
	else
	{
		auto data = cJSON_PrintUnformatted(root);
		lua_pushstring(state, data);
	}

	cJSON_Delete(root);

	return 1;
}

void luaRegisterJsonAPI(lua_State* state)
{
	lua_settop(state, 0);

	lua_newtable(state);

	lua_pushcfunction(state, decode);
	lua_setfield(state, -2, "decode");

	lua_pushcfunction(state, encode);
	lua_setfield(state, -2, "encode");

	lua_setglobal(state, "json");
}