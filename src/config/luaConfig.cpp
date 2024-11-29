#include<lua/luaApi.hpp>
#include<config/configMgr.hpp>
#include <stdexcept>

int32_t toInt(const std::string& str)
{
	try
	{
		return std::stoi(str);
	}
	catch (std::exception& e)
	{
		return 0;
	}
}

int32_t initConfig(lua_State* L)
{
	if (!lua_istable(L, 1))
	{
		lua_pushboolean(L, false);
		return 1;
	}

	auto configMgr = ConfigMgr::getInst();
	auto configs = ConfigMgrType();
	lua_pushnil(L);
	while (lua_next(L, 1))
	{
		if (!lua_istable(L, -1))
		{
			lua_pop(L, 1);
			continue;
		}

		auto configName = luaL_checkstring(L, -2);
		auto idConfigs = std::make_shared<ConfigMgrLineType>();

		lua_pushnil(L);
		while(lua_next(L, -2))
		{
			if (!lua_istable(L, -1))
			{
				lua_pop(L, 1);
				continue;
			}

			auto id = luaL_checkstring(L, -2);
			auto keyConfigs = std::make_shared<ConfigMgrKeyType>();

			lua_pushnil(L);
			while (lua_next(L, -2))
			{
				auto key = luaL_checkstring(L, -2);
				auto valueType = lua_type(L, -1);
				auto value = std::make_shared<Config>();
				switch (valueType)
				{
				case LUA_TBOOLEAN:
					value->intValue = lua_toboolean(L, -1);
					value->type = ConfigValueType::Bool;
					break;
				case LUA_TNUMBER:
					if (lua_isinteger(L, -1))
					{
						value->intValue = lua_tointeger(L, -1);
						value->type = ConfigValueType::Int;
					}
					else
					{
						value->floatValue = lua_tonumber(L, -1);
						value->type = ConfigValueType::Float;
					}
					break;
				case LUA_TSTRING:
					value->stringValue = lua_tostring(L, -1);
					value->type = ConfigValueType::String;
					break;
				case LUA_TTABLE:
					lua_getfield(L, -1, "data");
					value->stringValue=luaL_checkstring(L, -1);
					value->type = ConfigValueType::Json;
					lua_pop(L, 1);
					break;
				default:
					lua_pop(L, 1);
					continue;
				}
				lua_pop(L, 1);

				keyConfigs->insert({ key,value });
			}

			lua_pop(L, 1);

			idConfigs->insert({ id,keyConfigs });
		}

		lua_pop(L, 1);

		configs.insert({ configName ,idConfigs });
	}

	configMgr->mConfigs = configs;

	lua_pushboolean(L, true);
	return 1;
}

int32_t getConfig(lua_State* L)
{
	auto configName = std::string(luaL_checkstring(L, 1));
	auto configMgr = ConfigMgr::getInst();
	auto config = configMgr->getConfig(configName);
	if (!config)
	{
		return 0;
	}

	auto jsonMaps = std::unordered_map<std::string, std::vector<std::string>>();
	lua_newtable(L);
	for (auto iter = config->begin(); iter != config->end(); ++iter)
	{
		lua_newtable(L);
		for (auto it = iter->second->begin(); it != iter->second->end(); ++it)
		{
			switch (it->second->type)
			{
			case ConfigValueType::Bool:
				lua_pushboolean(L, it->second->intValue);
				break;
			case ConfigValueType::Int:
				lua_pushinteger(L, it->second->intValue);
				break;
			case ConfigValueType::Float:
				lua_pushinteger(L, it->second->floatValue);
				break;
			case ConfigValueType::String:
				lua_pushstring(L, it->second->stringValue.c_str());
				break;
			case ConfigValueType::Json:
			{
				lua_newtable(L);
				lua_pushstring(L, it->second->stringValue.c_str());
				lua_setfield(L, -2, "data");

				auto jsonMapIter = jsonMaps.find(iter->first);
				if (jsonMapIter == jsonMaps.end())
				{
					jsonMaps.insert({ iter->first,std::vector<std::string>() });
					jsonMapIter = jsonMaps.find(iter->first);
				}

				auto& jsonKeys = jsonMapIter->second;
				jsonKeys.emplace_back(it->first);
				break;
			}
			default:
				lua_pop(L, 1);
				continue;
			}

			lua_setfield(L, -2, it->first.c_str());
		}

		auto intField = toInt(iter->first);
		if (intField)
		{
			lua_rawseti(L, -2, intField);
		}
		else
		{
			lua_setfield(L, -2, iter->first.c_str());
		}
	}

	lua_newtable(L);
	for (auto it = jsonMaps.begin(); it != jsonMaps.end(); ++it)
	{
		lua_newtable(L);
		for (auto i = 0; i < it->second.size(); i++)
		{
			lua_pushstring(L, it->second[i].c_str());
			lua_rawseti(L, -2, i + 1);
		}

		auto intField = toInt(it->first);
		if (intField)
		{
			lua_rawseti(L, -2, intField);
		}
		else
		{
			lua_setfield(L, -2, it->first.c_str());
		}
	}

	return 2;
}

int32_t getConfigLine(lua_State* L)
{
	auto configName = std::string(luaL_checkstring(L, 1));
	auto id = std::string(luaL_checkstring(L, 2));

	auto configMgr = ConfigMgr::getInst();
	auto line = configMgr->getLine(configName, id);
	if (!line)
	{
		return 0;
	}

	auto jsonKeys = std::vector<std::string>();

	lua_newtable(L);
	for (auto it = line->begin(); it != line->end(); ++it)
	{
		switch (it->second->type)
		{
		case ConfigValueType::Bool:
			lua_pushboolean(L, it->second->intValue);
			break;
		case ConfigValueType::Int:
			lua_pushinteger(L, it->second->intValue);
			break;
		case ConfigValueType::Float:
			lua_pushinteger(L, it->second->floatValue);
			break;
		case ConfigValueType::String:
			lua_pushstring(L, it->second->stringValue.c_str());
			break;
		case ConfigValueType::Json:
			lua_newtable(L);
			lua_pushstring(L, it->second->stringValue.c_str());
			lua_setfield(L, -2, "data");
			jsonKeys.emplace_back(it->first);
			break;
		}

		lua_setfield(L, -2, it->first.c_str());
	}

	lua_newtable(L);
	for (auto i=0;i<jsonKeys.size();i++)
	{
		lua_pushstring(L, jsonKeys[i].c_str());
		lua_rawseti(L, -2, i + 1);
	}

	return 2;
}

int32_t getConfigValue(lua_State* L)
{
	auto configName = std::string(luaL_checkstring(L, 1));
	auto id = std::string(luaL_checkstring(L, 2));
	auto key = std::string(luaL_checkstring(L, 3));

	auto configMgr = ConfigMgr::getInst();
	auto value = configMgr->getValue(configName, id, key);
	if (!value)
	{
		return 0;
	}

	switch (value->type)
	{
	case ConfigValueType::Bool:
		lua_pushboolean(L, value->intValue);
		break;
	case ConfigValueType::Int:
		lua_pushinteger(L, value->intValue);
		break;
	case ConfigValueType::Float:
		lua_pushinteger(L, value->floatValue);
		break;
	case ConfigValueType::String:
		lua_pushstring(L, value->stringValue.c_str());
		break;
	case ConfigValueType::Json:
		lua_newtable(L);
		lua_pushstring(L, value->stringValue.c_str());
		lua_setfield(L, -2, "data");
		break;
	default:
		break;
	}

	return 1;
}

void luaRegisterConfigAPI(lua_State* state)
{
	lua_settop(state, 0);

	lua_newtable(state);

	lua_pushcfunction(state, initConfig);
	lua_setfield(state, -2, "init");

	lua_pushcfunction(state, getConfig);
	lua_setfield(state, -2, "getConfig");

	lua_pushcfunction(state, getConfigLine);
	lua_setfield(state, -2, "getLine");

	lua_pushcfunction(state, getConfigValue);
	lua_setfield(state, -2, "getValue");

	lua_setglobal(state, "config");
}