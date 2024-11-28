#include<lua/luaApi.hpp>
#include<memory>
#include<vector>
#include<fstream>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/dynamic_message.h>
#include<spdlog/spdlog.h>

struct ProtoPtr
{
	google::protobuf::DescriptorPool* descriptorPool;
	google::protobuf::DynamicMessageFactory* factory;
};

int32_t newProto(lua_State* L)
{
	auto ptr = new ProtoPtr();
	ptr->descriptorPool = new google::protobuf::DescriptorPool();
	ptr->factory = new google::protobuf::DynamicMessageFactory();
	lua_pushlightuserdata(L, ptr);
	return 1;
}

int32_t destroyProto(lua_State* L)
{
	auto ptr = static_cast<ProtoPtr*>(lua_touserdata(L, 1));
	if (ptr->factory)
	{
		delete ptr->factory;
		ptr->factory = nullptr;
	}

	if (ptr->descriptorPool)
	{
		delete ptr->descriptorPool;
		ptr->descriptorPool = nullptr;
	}

	delete ptr;
	ptr = nullptr;

	return 0;
}

int32_t loadfileProto(lua_State* L)
{
	auto ptr = static_cast<ProtoPtr*>(lua_touserdata(L, 1));
	auto filePath = luaL_checkstring(L, 2);
	auto file = std::ifstream(filePath, std::ios::binary);
	if (!file.is_open())
	{
		lua_pushboolean(L, false);
		return 1;
	}

	auto fileDescriptorSet = google::protobuf::FileDescriptorSet();
	if (!fileDescriptorSet.ParseFromIstream(&file))
	{
		lua_pushboolean(L, false);
		return 1;
	}
	file.close();

	for (int i = 0; i < fileDescriptorSet.file_size(); ++i)
	{
		auto& protoFile = fileDescriptorSet.file(i);
		if (!ptr->descriptorPool->BuildFile(protoFile))
		{
			spdlog::error("failed to build proto:{}", protoFile.name());
			lua_pushboolean(L, false);
			return 1;
		}
	}

	lua_pushboolean(L, true);
	return 1;
}

void foreachToMsg(lua_State* L, ProtoPtr* ptr,google::protobuf::Message* msg)
{
	if (!lua_istable(L, -1))
	{
		return;
	}

	auto descriptor = msg->GetDescriptor();
	auto reflection = msg->GetReflection();
	for (auto i = 0; i < descriptor->field_count(); i++)
	{
		auto field = descriptor->field(i);
		auto fieldType = field->type();
		lua_getfield(L, -1, field->name().c_str());
		auto luaType = lua_type(L, -1);

		if (field->is_repeated())
		{
			if (luaType == LUA_TTABLE)
			{
				auto count = luaL_len(L, -1);

				for (auto i = 0; i < count; i++)
				{
					lua_rawgeti(L, -1, i+1);
					auto type = lua_type(L, -1);
					switch (fieldType)
					{
					case google::protobuf::FieldDescriptor::Type::TYPE_BOOL:
						if (type == LUA_TBOOLEAN)
						{
							reflection->AddBool(msg, field, lua_toboolean(L, -1));
						}
						break;
					case google::protobuf::FieldDescriptor::Type::TYPE_INT32:
					case google::protobuf::FieldDescriptor::Type::TYPE_SFIXED32:
					case google::protobuf::FieldDescriptor::Type::TYPE_SINT32:
						if (type == LUA_TNUMBER)
						{
							reflection->AddInt32(msg, field, lua_tointeger(L, -1));
						}
						break;
					case google::protobuf::FieldDescriptor::Type::TYPE_INT64:
					case google::protobuf::FieldDescriptor::Type::TYPE_SFIXED64:
					case google::protobuf::FieldDescriptor::Type::TYPE_SINT64:
						if (type == LUA_TNUMBER)
						{
							reflection->AddInt64(msg, field, lua_tointeger(L, -1));
						}
						break;
					case google::protobuf::FieldDescriptor::Type::TYPE_FIXED32:
					case google::protobuf::FieldDescriptor::Type::TYPE_UINT32:
						if (type == LUA_TNUMBER)
						{
							reflection->AddUInt32(msg, field, lua_tointeger(L, -1));
						}
						break;
					case google::protobuf::FieldDescriptor::Type::TYPE_UINT64:
					case google::protobuf::FieldDescriptor::Type::TYPE_FIXED64:
						if (type == LUA_TNUMBER)
						{
							reflection->AddUInt64(msg, field, lua_tointeger(L, -1));
						}
						break;
					case google::protobuf::FieldDescriptor::Type::TYPE_FLOAT:
						if (type == LUA_TNUMBER)
						{
							reflection->AddFloat(msg, field, lua_tonumber(L, -1));
						}
						break;
					case google::protobuf::FieldDescriptor::Type::TYPE_DOUBLE:
						if (type == LUA_TNUMBER)
						{
							reflection->AddDouble(msg, field, lua_tonumber(L, -1));
						}
						break;
					case google::protobuf::FieldDescriptor::Type::TYPE_STRING:
						if (type == LUA_TSTRING)
						{
							reflection->AddString(msg, field, lua_tostring(L, -1));
						}
						break;
					case google::protobuf::FieldDescriptor::Type::TYPE_MESSAGE:
					{
						if (type == LUA_TTABLE)
						{
							auto protoType = ptr->factory->GetPrototype(field->message_type());
							if (!protoType)
							{
								break;
							}

							auto newMsg = protoType->New();
							foreachToMsg(L, ptr, newMsg);
							reflection->AddAllocatedMessage(msg, field, newMsg);
						}
						break;
					}
					default:
						spdlog::error("unsupport proto field type:{}", field->type_name());
						break;
					}
					lua_pop(L, 1);
				}
			}
		}
		else
		{
			switch (fieldType)
			{
			case google::protobuf::FieldDescriptor::Type::TYPE_BOOL:
				if (luaType == LUA_TBOOLEAN)
				{
					reflection->SetBool(msg, field, lua_toboolean(L, -1));
				}
				break;
			case google::protobuf::FieldDescriptor::Type::TYPE_INT32:
			case google::protobuf::FieldDescriptor::Type::TYPE_SFIXED32:
			case google::protobuf::FieldDescriptor::Type::TYPE_SINT32:
				if (luaType == LUA_TNUMBER)
				{
					reflection->SetInt32(msg, field, lua_tointeger(L, -1));
				}
				break;
			case google::protobuf::FieldDescriptor::Type::TYPE_INT64:
			case google::protobuf::FieldDescriptor::Type::TYPE_SFIXED64:
			case google::protobuf::FieldDescriptor::Type::TYPE_SINT64:
				if (luaType == LUA_TNUMBER)
				{
					reflection->SetInt64(msg, field, lua_tointeger(L, -1));
				}
				break;
			case google::protobuf::FieldDescriptor::Type::TYPE_FIXED32:
			case google::protobuf::FieldDescriptor::Type::TYPE_UINT32:
				if (luaType == LUA_TNUMBER)
				{
					reflection->SetUInt32(msg, field, lua_tointeger(L, -1));
				}
				break;
			case google::protobuf::FieldDescriptor::Type::TYPE_UINT64:
			case google::protobuf::FieldDescriptor::Type::TYPE_FIXED64:
				if (luaType == LUA_TNUMBER)
				{
					reflection->SetUInt64(msg, field, lua_tointeger(L, -1));
				}
				break;
			case google::protobuf::FieldDescriptor::Type::TYPE_FLOAT:
				if (luaType == LUA_TNUMBER)
				{
					reflection->SetFloat(msg, field, lua_tonumber(L, -1));
				}
				break;
			case google::protobuf::FieldDescriptor::Type::TYPE_DOUBLE:
				if (luaType == LUA_TNUMBER)
				{
					reflection->SetDouble(msg, field, lua_tonumber(L, -1));
				}
				break;
			case google::protobuf::FieldDescriptor::Type::TYPE_STRING:
				if (luaType == LUA_TSTRING)
				{
					reflection->SetString(msg, field, lua_tostring(L, -1));
				}
				break;
			case google::protobuf::FieldDescriptor::Type::TYPE_MESSAGE:
			{
				auto protoType =ptr->factory->GetPrototype(field->message_type());
				if (!protoType)
				{
					return;
				}
				
				auto newMsg= protoType->New();
				foreachToMsg(L, ptr, newMsg);
				reflection->SetAllocatedMessage(msg, newMsg, field);
				break;
			}
			default:
				spdlog::error("unsupport proto field type:{}", field->type_name());
				break;
			}
		}
		lua_pop(L, 1);
	}
}

void foreachToTable(lua_State* L, ProtoPtr* ptr, google::protobuf::Message* msg)
{
	auto descriptor = msg->GetDescriptor();
	auto reflection = msg->GetReflection();

	for (auto i = 0; i < descriptor->field_count(); i++)
	{
		auto field = descriptor->field(i);
		auto fieldType = field->type();
		if (field->is_repeated())
		{
			auto count = reflection->FieldSize(*msg, field);
			lua_newtable(L);
			auto flag = 1;
			for (auto j = 0; j < count; j++)
			{
				switch (fieldType)
				{
				case google::protobuf::FieldDescriptor::Type::TYPE_BOOL:
					lua_pushboolean(L, reflection->GetRepeatedBool(*msg, field, j));
					break;
				case google::protobuf::FieldDescriptor::Type::TYPE_INT32:
				case google::protobuf::FieldDescriptor::Type::TYPE_SFIXED32:
				case google::protobuf::FieldDescriptor::Type::TYPE_SINT32:
					lua_pushinteger(L, reflection->GetRepeatedInt32(*msg, field, j));
					break;
				case google::protobuf::FieldDescriptor::Type::TYPE_INT64:
				case google::protobuf::FieldDescriptor::Type::TYPE_SFIXED64:
				case google::protobuf::FieldDescriptor::Type::TYPE_SINT64:
					lua_pushinteger(L, reflection->GetRepeatedInt64(*msg, field, j));
					break;
				case google::protobuf::FieldDescriptor::Type::TYPE_FIXED32:
				case google::protobuf::FieldDescriptor::Type::TYPE_UINT32:
					lua_pushinteger(L, reflection->GetRepeatedUInt32(*msg, field,j));
					break;
				case google::protobuf::FieldDescriptor::Type::TYPE_UINT64:
				case google::protobuf::FieldDescriptor::Type::TYPE_FIXED64:
					lua_pushinteger(L, reflection->GetRepeatedUInt64(*msg, field,j));
					break;
				case google::protobuf::FieldDescriptor::Type::TYPE_FLOAT:
					lua_pushnumber(L, reflection->GetRepeatedFloat(*msg, field,j));
					break;
				case google::protobuf::FieldDescriptor::Type::TYPE_DOUBLE:
					lua_pushnumber(L, reflection->GetRepeatedDouble(*msg, field,j));
					break;
				case google::protobuf::FieldDescriptor::Type::TYPE_STRING:
					lua_pushstring(L, reflection->GetRepeatedString(*msg, field,j).c_str());
					break;
				case google::protobuf::FieldDescriptor::Type::TYPE_MESSAGE:
				{
					lua_newtable(L);
					auto newMsg = reflection->MutableRepeatedMessage(msg, field, j);
					foreachToTable(L, ptr, newMsg);
					break;
				}
				default:
					spdlog::error("unsupport proto field type:{}", field->type_name());
					continue;
				}
				lua_rawseti(L, -2, flag);
				flag++;
			}
		}
		else
		{
			switch (fieldType)
			{
			case google::protobuf::FieldDescriptor::Type::TYPE_BOOL:
				lua_pushboolean(L, reflection->GetBool(*msg, field));
				break;
			case google::protobuf::FieldDescriptor::Type::TYPE_INT32:
			case google::protobuf::FieldDescriptor::Type::TYPE_SFIXED32:
			case google::protobuf::FieldDescriptor::Type::TYPE_SINT32:
				lua_pushinteger(L, reflection->GetInt32(*msg, field));
				break;
			case google::protobuf::FieldDescriptor::Type::TYPE_INT64:
			case google::protobuf::FieldDescriptor::Type::TYPE_SFIXED64:
			case google::protobuf::FieldDescriptor::Type::TYPE_SINT64:
				lua_pushinteger(L, reflection->GetInt64(*msg, field));
				break;
			case google::protobuf::FieldDescriptor::Type::TYPE_FIXED32:
			case google::protobuf::FieldDescriptor::Type::TYPE_UINT32:
				lua_pushinteger(L, reflection->GetUInt32(*msg, field));
				break;
			case google::protobuf::FieldDescriptor::Type::TYPE_UINT64:
			case google::protobuf::FieldDescriptor::Type::TYPE_FIXED64:
				lua_pushinteger(L, reflection->GetUInt64(*msg, field));
				break;
			case google::protobuf::FieldDescriptor::Type::TYPE_FLOAT:
				lua_pushnumber(L, reflection->GetFloat(*msg, field));
				break;
			case google::protobuf::FieldDescriptor::Type::TYPE_DOUBLE:
				lua_pushnumber(L, reflection->GetDouble(*msg, field));
				break;
			case google::protobuf::FieldDescriptor::Type::TYPE_STRING:
				lua_pushstring(L, reflection->GetString(*msg, field).c_str());
				break;
			case google::protobuf::FieldDescriptor::Type::TYPE_MESSAGE:
			{
				auto newMsg = reflection->MutableMessage(msg, field);
				lua_newtable(L);
				foreachToTable(L, ptr, newMsg);
				break;
			}
			default:
				spdlog::error("unsupport proto field type:{}", field->type_name());
				continue;
			}
		}
		lua_setfield(L, -2, field->name().c_str());
	}
}

int32_t encodeProto(lua_State* L)
{
	auto ptr = static_cast<ProtoPtr*>(lua_touserdata(L, 1));
	auto msgTypeName = luaL_checkstring(L, 2);
	if (!lua_istable(L, 3))
	{
		lua_pushboolean(L, false);
		return 0;
	}

	auto descriptor = ptr->descriptorPool->FindMessageTypeByName(msgTypeName);
	if (!descriptor)
	{
		spdlog::error("proto message type not found:{}", msgTypeName);
		lua_pushboolean(L, false);
		return 1;
	}

	auto protoType = ptr->factory->GetPrototype(descriptor);
	if (!protoType)
	{
		spdlog::error("failed to create message protoType for:{}", msgTypeName);
		lua_pushboolean(L, false);
		return 1;
	}

	auto msg = std::unique_ptr<google::protobuf::Message>(protoType->New());
	foreachToMsg(L, ptr, msg.get());

	auto data = std::vector<uint8_t>(msg->ByteSize());
	if (!msg->SerializeToArray(data.data(), data.size()))
	{
		spdlog::error("proto encode failed!msgType:{}", msgTypeName);
		lua_pushboolean(L, false);
		return 1;
	}
	lua_pushboolean(L,true);
	lua_pushlstring(L, reinterpret_cast<const char*>(data.data()), data.size());

	return 2;
}

int32_t decodeProto(lua_State* L)
{
	auto ptr = static_cast<ProtoPtr*>(lua_touserdata(L, 1));
	auto msgTypeName = luaL_checkstring(L, 2);
	auto data = luaL_checkstring(L, 3);
	auto dataLen = luaL_len(L, 3);

	auto descriptor = ptr->descriptorPool->FindMessageTypeByName(msgTypeName);
	if (!descriptor)
	{
		spdlog::error("message type not found:{}", msgTypeName);
		lua_pushboolean(L, false);
		return 1;
	}

	auto protoType = ptr->factory->GetPrototype(descriptor);
	if (!protoType)
	{
		spdlog::error("failed to create message protoType for:{}", msgTypeName);
		lua_pushboolean(L, false);
		return 1;
	}

	auto msg = std::unique_ptr<google::protobuf::Message>(protoType->New());
	if (!msg->ParseFromArray(data, dataLen))
	{
		spdlog::error("parse message failed!protoType:{}", msgTypeName);
		lua_pushboolean(L, false);
		return 1;
	}

	lua_pushboolean(L, true);
	lua_newtable(L);
	foreachToTable(L, ptr, msg.get());

	return 2;
}

void luaRegisterProtobufAPI(lua_State* state)
{
	lua_settop(state, 0);

	lua_newtable(state);

	lua_pushcfunction(state, newProto);
	lua_setfield(state, -2, "new");

	lua_pushcfunction(state, destroyProto);
	lua_setfield(state, -2, "destroy");

	lua_pushcfunction(state, loadfileProto);
	lua_setfield(state, -2, "loadFile");

	lua_pushcfunction(state, encodeProto);
	lua_setfield(state, -2, "encode");

	lua_pushcfunction(state, decodeProto);
	lua_setfield(state, -2, "decode");

	lua_setglobal(state, "proto");
}