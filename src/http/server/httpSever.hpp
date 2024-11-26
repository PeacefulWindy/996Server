#pragma once
#include<string>
#include<ixwebsocket/IXHttpServer.h>
#include<functional>

struct HttpServerRequest
{
	std::string method;
	std::string version;
	std::string body;
	std::map<std::string, std::string> headers;
};

struct HttpServerResponse
{
	int32_t status = 200;
	std::string msg;
};

class HttpServer
{
public:
	HttpServer();
	virtual ~HttpServer();

public:
	bool listen(int32_t port);

public:
	void on(std::string url, std::function<std::shared_ptr<HttpServerResponse>(std::shared_ptr<HttpServerRequest>)> func);
	void setOnMsgFunc(std::function < std::shared_ptr<HttpServerResponse>(std::shared_ptr<HttpServerRequest>)> func);

public:
	void close();

private:
	ix::HttpResponsePtr onMsg(ix::HttpRequestPtr request, std::shared_ptr<ix::ConnectionState> connectionState);

private:
	std::map<std::string, std::function<std::shared_ptr<HttpServerResponse>(std::shared_ptr<HttpServerRequest>)>> mRouters;
	std::function<std::shared_ptr<HttpServerResponse>(std::shared_ptr<HttpServerRequest>)> mOnMsgFunc;
	ix::HttpServer* mServer=nullptr;
};