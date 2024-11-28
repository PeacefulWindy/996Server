#include<lua/luaApi.hpp>
#include<cryptopp/cryptlib.h>
#include<cryptopp/osrng.h>
#include<cryptopp/chacha.h>

int chacha20_gen(lua_State* L)
{
	auto rng = CryptoPP::AutoSeededRandomPool();
	auto key = CryptoPP::SecByteBlock(CryptoPP::ChaCha::DEFAULT_KEYLENGTH);
	auto iv = CryptoPP::SecByteBlock(CryptoPP::ChaCha::IV_LENGTH);

	rng.GenerateBlock(key, key.size());
	rng.GenerateBlock(iv, iv.size());

	return 0;
}