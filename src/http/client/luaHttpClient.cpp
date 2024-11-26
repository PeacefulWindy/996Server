#include<lua/luaApi.hpp>
#include<ixwebsocket/IXHttpClient.h>
#include<spdlog/spdlog.h>
#include<http/client/httpClientMgr.hpp>
#include <service/service.hpp>
#include <service/serviceMgr.hpp>

constexpr int32_t HttpConnectTimeOut = 10;
constexpr int32_t HttpTransferTimeOut = 60;

void packRequestArgs(ix::HttpRequestArgsPtr request,lua_State* L,int32_t index)
{
	lua_pushnil(L);
	while (lua_next(L, index))
	{
		auto key = luaL_checkstring(L, -2);
		if (strcmp(key, "headers") == 0)
		{
			auto argIndex = lua_gettop(L);
			lua_pushnil(L);

			while (lua_next(L, argIndex))
			{
				auto headerKey = luaL_checkstring(L, -2);
				auto headerValue = luaL_checkstring(L, -1);
				request->extraHeaders.insert({ headerKey,headerValue });
				lua_pop(L, 1);
			}
		}
		else if (strcmp(key, "url") == 0)
		{
			request->url = luaL_checkstring(L, -1);
		}

		lua_pop(L, 1);
	}
}

int32_t httpGet(lua_State* L)
{
	auto sessionId = lua_tointeger(L, 1);
	if (!lua_istable(L, 2))
	{
		lua_pushboolean(L, false);
		return 1;
	}

	auto httpClientMgr = HttpClientMgr::getInst();
	auto clientId = httpClientMgr->newClient();
	auto client = httpClientMgr->getClient(clientId);
	auto request = client->createRequest();
	request->connectTimeout = HttpConnectTimeOut;
	request->transferTimeout = HttpTransferTimeOut;

	packRequestArgs(request, L, 2);

	if (request->url.empty())
	{
		lua_pushboolean(L, false);
		return 1;
	}

	lua_getglobal(L, "SERVICE_ID");
	auto serviceId = luaL_checkinteger(L, -1);

	auto ret = client->performRequest(request, [sessionId, serviceId, clientId](const ix::HttpResponsePtr& response)
		{
			auto msg = std::make_shared<ServiceMsg>();
			msg->msgType = static_cast<uint32_t>(ServiceMsgType::HttpResponse);
			msg->session = sessionId;
			msg->httpResponse.status = response->statusCode;
			msg->httpResponse.body = response->body;
			msg->httpResponse.error = response->errorMsg;

			ServiceMgr::getInst()->send(serviceId, msg);

			auto httpClientMgr = HttpClientMgr::getInst();
			httpClientMgr->destroyClient(clientId);
		});

	if (!ret)
	{
		httpClientMgr->destroyClient(clientId);
	}

	lua_pushboolean(L, ret);

	return 1;
}

int httpPost(lua_State* L)
{
	auto sessionId = lua_tointeger(L, 1);
	if (!lua_istable(L, 2)||!lua_istable(L, 3))
	{
		lua_pushboolean(L, false);
		return 1;
	}

	auto httpClientMgr = HttpClientMgr::getInst();
	auto clientId = httpClientMgr->newClient();
	auto client = httpClientMgr->getClient(clientId);
	auto request = client->createRequest();
	request->verb = ix::HttpClient::kPost;
	request->connectTimeout = HttpConnectTimeOut;
	request->transferTimeout = HttpTransferTimeOut;

	auto form=ix::HttpFormDataParameters();

	lua_pushnil(L);
	while (lua_next(L, 2))
	{
		auto key = luaL_checkstring(L, -2);
		auto value = luaL_checkstring(L, -1);

		form.insert({ key,value });

		lua_pop(L, 1);
	}

	auto multipartBoundary = client->generateMultipartBoundary();
	request->body = client->serializeHttpFormDataParameters(multipartBoundary, form);

	packRequestArgs(request, L, 3);

	if (request->url.empty())
	{
		lua_pushboolean(L, false);
		return 1;
	}

	lua_getglobal(L, "SERVICE_ID");
	auto serviceId = luaL_checkinteger(L, -1);
	
	auto ret = client->performRequest(request, [sessionId, serviceId, clientId](const ix::HttpResponsePtr& response)
		{
			auto msg = std::make_shared<ServiceMsg>();
			msg->msgType = static_cast<uint32_t>(ServiceMsgType::HttpResponse);
			msg->session = sessionId;
			msg->httpResponse.status = response->statusCode;
			msg->httpResponse.body = response->body;
			msg->httpResponse.error = response->errorMsg;

			ServiceMgr::getInst()->send(serviceId, msg);

			auto httpClientMgr = HttpClientMgr::getInst();
			httpClientMgr->destroyClient(clientId);
		});

	if (!ret)
	{
		httpClientMgr->destroyClient(clientId);
	}

	lua_pushboolean(L, ret);

	return 1;
}

void luaRegisterHttpClientAPI(lua_State* state)
{
	lua_settop(state, 0);

	lua_newtable(state);

	lua_pushcfunction(state, httpGet);
	lua_setfield(state, -2, "get");

	lua_pushcfunction(state, httpPost);
	lua_setfield(state, -2, "post");

	lua_setglobal(state, "httpClient");
}