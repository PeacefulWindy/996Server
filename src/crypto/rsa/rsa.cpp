#include<lua/luaApi.hpp>
#include<cryptopp/cryptlib.h>
#include<cryptopp/rsa.h>
#include<cryptopp/base64.h>
#include<cryptopp/osrng.h>
#include<cryptopp/files.h>
#include<crypto/util/util.hpp>

int rsa_gen(lua_State* L)
{
	auto keyLen = luaL_checkinteger(L, 1);

	auto rng = CryptoPP::AutoSeededRandomPool();
	auto privateKey = CryptoPP::InvertibleRSAFunction();
	privateKey.Initialize(rng, keyLen);

	auto privateKeyData = std::string();
	auto privateKeySink = CryptoPP::Base64Encoder(new CryptoPP::StringSink(privateKeyData));
	privateKey.DEREncode(privateKeySink);
	privateKeySink.MessageEnd();

	auto publicKey = CryptoPP::RSAFunction(privateKey);
	auto publicKeyData = std::string();
	auto pulbicKeySink = CryptoPP::Base64Encoder(new CryptoPP::StringSink(publicKeyData));
	publicKey.DEREncode(pulbicKeySink);
	pulbicKeySink.MessageEnd();

	lua_pushlstring(L, publicKeyData.c_str(), publicKeyData.length());
	lua_pushlstring(L, privateKeyData.c_str(), privateKeyData.length());

	return 2;
}

int rsa_decode(lua_State* L)
{
	auto top = lua_gettop(L);
	auto keyData = std::string(luaL_checkstring(L, 1));
	auto hexInput = luaL_checkstring(L, 2);

	auto hexInputLen = 0;
	if (top > 2)
	{
		hexInputLen = luaL_checknumber(L, 3);
	}
	else
	{
		hexInputLen = strlen(hexInput);
	}

	auto key = CryptoPP::RSA::PrivateKey();
	auto keySource = CryptoPP::StringSource(keyData, true, new CryptoPP::Base64Decoder);

	try
	{
		key.BERDecode(keySource);
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return 0;
	}

	auto encryptor = CryptoPP::RSAES_OAEP_SHA_Decryptor(key);
	auto outputText = std::string();
	auto rng = CryptoPP::AutoSeededRandomPool();

	try
	{
		auto input = hexToData(std::string(hexInput, hexInputLen));
		CryptoPP::StringSource(input.data(), input.size(), true, new CryptoPP::PK_DecryptorFilter(rng, encryptor, new CryptoPP::StringSink(outputText)));
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return 0;
	}

	lua_pushlstring(L, outputText.c_str(), outputText.length());
	return 1;
}

int rsa_encode(lua_State* L)
{
	auto top = lua_gettop(L);
	auto keyData = std::string(luaL_checkstring(L, 1));
	auto input = luaL_checkstring(L, 2);

	auto inputLen = 0;
	if (top > 2)
	{
		inputLen = luaL_checkinteger(L, 3);
	}
	else
	{
		inputLen = strlen(input);
	}


	auto key = CryptoPP::RSA::PublicKey();
	auto keySource = CryptoPP::StringSource(keyData, true, new CryptoPP::Base64Decoder);

	try
	{
		key.BERDecode(keySource);
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return 0;
	}

	auto encryptor = CryptoPP::RSAES_OAEP_SHA_Encryptor(key);
	auto rng = CryptoPP::AutoSeededRandomPool();
	auto outputText = std::string();

	try
	{
		auto output = std::string();
		CryptoPP::StringSource(reinterpret_cast<const CryptoPP::byte*>(input), inputLen, true, new CryptoPP::PK_EncryptorFilter(rng, encryptor, new CryptoPP::StringSink(output)));
		outputText = dataToHex(output.c_str(), output.length());
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return 0;
	}

	lua_pushlstring(L, outputText.c_str(), outputText.length());
	return 1;
}

void luaRegisterCryptoRsaAPI(lua_State* state)
{
	lua_settop(state, 0);

	auto name = "crypto";
	auto hasTable = true;
	lua_getglobal(state, name);
	if (lua_isnil(state, -1))
	{
		lua_newtable(state);
		hasTable = false;
	}

	lua_pushcfunction(state, rsa_gen);
	lua_setfield(state, -2, "rsaGen");

	lua_pushcfunction(state, rsa_encode);
	lua_setfield(state, -2, "rsaEncode");

	lua_pushcfunction(state, rsa_decode);
	lua_setfield(state, -2, "rsaDecode");

	if (!hasTable)
	{
		lua_setglobal(state, name);
	}
	else
	{
		lua_pop(state, 1);
	}
}