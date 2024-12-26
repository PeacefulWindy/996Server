#include<libxml/parser.h>
#include<libxml/xmlwriter.h>
#include<lua.hpp>
#include<cstdint>
#include <spdlog/spdlog.h>
#include <lua/luaApi.hpp>

void fetchCreateLuaTable(lua_State* L, xmlNode* node)
{
	auto index = 1;
	for (auto* it = node; it; it = it->next)
	{
		if (node->type == XML_ELEMENT_NODE || node->type == XML_TEXT_NODE)
		{
			if (strcmp(reinterpret_cast<const char*>(it->name),"text") == 0)
			{
				auto data = reinterpret_cast<const char*>(it->content);
				auto dataLen = strlen(data);
				auto chIndex = -1;
				auto chLastIndex = -1;

				for (auto i = 0; i < dataLen; i++)
				{
					auto ch = data[i];
					if (ch != '\n' && ch != ' ')
					{
						chIndex = i;
						break;
					}
				}

				for (int64_t i = dataLen - 1; i >= 0; i--)
				{
					auto ch = data[i];
					if (ch != '\n' && ch != ' ' && ch != '\0')
					{
						chLastIndex = i + 1;
						break;
					}
				}

				if (chIndex > -1 && chLastIndex > -1)
				{
					auto len = chLastIndex - chIndex;
					if (len > 0)
					{
						lua_pushlstring(L, data + sizeof(char) * chIndex, len);
						lua_setfield(L, -2, "data");
					}
				}
			}
			else
			{
				lua_newtable(L);
				lua_pushstring(L, reinterpret_cast<const char*>(it->name));
				lua_setfield(L, -2, "name");

				if (it->properties)
				{
					lua_newtable(L);
					for (auto attr = it->properties; attr; attr = attr->next)
					{
						lua_pushstring(L, reinterpret_cast<const char*>(xmlGetProp(it, attr->name)));
						lua_setfield(L, -2, reinterpret_cast<const char*>(attr->name));
					}
					lua_setfield(L, -2, "attrs");
				}

				if (it->children)
				{
					lua_newtable(L);
					fetchCreateLuaTable(L, it->children);
					lua_setfield(L, -2, "children");
				}

				lua_rawseti(L, -2, index);
				index++;
			}
		}
		else
		{
			spdlog::warn("unsupport xml type:{}", static_cast<int32_t>(node->type));
		}
	}
}

int32_t xmlDecode(lua_State* L)
{
	auto dataLen = static_cast<size_t>(luaL_len(L, 1));
	auto data = luaL_checklstring(L, 1, &dataLen);

	xmlInitParser();
	auto doc = xmlReadMemory(data, dataLen, nullptr, nullptr, XML_PARSE_NOBLANKS);
	if (!doc)
	{
		xmlCleanupParser();
		return 0;
	}

	auto root = xmlDocGetRootElement(doc);

	lua_newtable(L);
	fetchCreateLuaTable(L, root);

	xmlFreeDoc(doc);
	xmlCleanupParser();
	return 1;
}

void fetchCreateXml(lua_State* L, xmlTextWriterPtr writer,int32_t tableIndex)
{
	lua_pushnil(L);
	while (lua_next(L, tableIndex) != 0)
	{
		if (lua_istable(L,-1))
		{
			lua_getfield(L, -1, "name");
			if (!lua_isstring(L, -1))
			{
				lua_pop(L, 1);
				lua_pop(L, 1);
				continue;
			}

			xmlTextWriterStartElement(writer, reinterpret_cast<const xmlChar*>(lua_tostring(L, -1)));
			lua_pop(L, 1);

			lua_getfield(L, -1, "attrs");
			if (lua_istable(L, -1))
			{
				auto attrIndex = lua_gettop(L);
				lua_pushnil(L);
				while (lua_next(L, attrIndex) != 0)
				{
					xmlTextWriterWriteAttribute(writer, reinterpret_cast<const xmlChar*>(luaL_checkstring(L, -2)), reinterpret_cast<const xmlChar*>(luaL_checkstring(L, -1)));
					lua_pop(L, 1);
				}
			}
			lua_pop(L, 1);

			lua_getfield(L, -1, "data");
			if (lua_isstring(L, -1))
			{
				xmlTextWriterWriteString(writer, reinterpret_cast<const xmlChar*>(lua_tostring(L, -1)));
			}
			lua_pop(L, 1);

			lua_getfield(L, -1, "children");
			if (lua_istable(L,-1))
			{
				auto index = lua_gettop(L);
				fetchCreateXml(L, writer, index);
			}
			lua_pop(L, 1);

			xmlTextWriterEndElement(writer);
		}

		lua_pop(L, 1);
	}
}

int32_t xmlEncode(lua_State* L)
{
	if (!lua_istable(L, 1))
	{
		return 0;
	}

	auto buf = xmlBufferCreate();
	if (!buf)
	{
		spdlog::error("create xml buffer error");
		return 0;
	}

	auto writer = xmlNewTextWriterMemory(buf, 0);
	if (!writer)
	{
		spdlog::error("create xml writer error");
		xmlBufferFree(buf);
		return 0;
	}

	xmlTextWriterStartDocument(writer, "1.0", "UTF-8", nullptr);
	fetchCreateXml(L, writer, 1);
	xmlTextWriterEndDocument(writer);
	xmlFreeTextWriter(writer);

	lua_pushstring(L, reinterpret_cast<const char*>(xmlBufferContent(buf)));
	xmlBufferFree(buf);

	return 1;
}

void luaRegisterXmlAPI(lua_State* state)
{
	lua_settop(state, 0);

	lua_newtable(state);

	lua_pushcfunction(state, xmlDecode);
	lua_setfield(state, -2, "decode");

	lua_pushcfunction(state, xmlEncode);
	lua_setfield(state, -2, "encode");

	lua_setglobal(state, "xml");
}