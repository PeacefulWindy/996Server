#include<lua/luaApi.hpp>
#include<stduuid/uuid.h>
#include <cassert>

int uuid(lua_State* L)
{
	auto rd = std::random_device();
	auto seed_data = std::array<int, std::mt19937::state_size> {};
	std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
	auto seq = std::seed_seq(std::begin(seed_data), std::end(seed_data));
	auto generator = std::mt19937(seq);
	auto gen = uuids::uuid_random_generator{ generator };

	const auto id = gen();
	assert(!id.is_nil());
	assert(id.as_bytes().size() == 16);
	assert(id.version() == uuids::uuid_version::random_number_based);
	assert(id.variant() == uuids::uuid_variant::rfc);

	auto ret = uuids::to_string(id);
	lua_pushstring(L, ret.c_str());

	return 1;
}

void luaRegisterUUIDAPI(lua_State* state)
{
	lua_settop(state, 0);

	lua_newtable(state);

	lua_pushcfunction(state, uuid);
	lua_setfield(state, -2, "uuid");

	lua_setglobal(state, "uuid");
}